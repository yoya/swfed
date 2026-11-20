--TEST--
www/define.php: the player that the demo pages load (Flapp or none)
--SKIPIF--
<?php
// www/ is outside of the extension source: not there in some builds (Windows)
if (! file_exists(__DIR__ . '/../../www/define.php')) {
    die('skip no www/');
}
?>
--FILE--
<?php

$dir = sys_get_temp_dir() . '/swfed-www001-' . getmypid();
putenv("SWFED_TMP_DIR=$dir");
ini_set('error_log', "$dir.log");
require __DIR__ . '/../../www/define.php';

function setenv($name, $value) {
    if ($value === null) {
        putenv($name); // unset
    } else {
        putenv("$name=$value");
    }
}
function has($s, $needle) {
    return (strpos($s, $needle) !== false) ? 'yes' : 'no';
}
$object = '<object>for Flash Player</object>';
$src = './swf.php?id=1&shape_id=2';

echo "== flapp (default)\n";
setenv('SWFED_PLAYER', null);
setenv('SWFED_PLAYER_URL', null);
$head = swfed_player_head();
echo swfed_player(), ": module=", has($head, '<script type="module" src="https://cdn.jsdelivr.net/gh/yoya/Flapp@'),
     " pinned to a commit=", preg_match('#/gh/yoya/Flapp@[0-9a-f]{40}/web/flapp\.js"#', $head) ? 'yes' : 'no', "\n";
echo swfed_swf_embed($src, $object), "\n";

echo "== flapp (not case sensitive)\n";
setenv('SWFED_PLAYER', 'Flapp');
echo swfed_player(), "\n";

echo "== none\n";
setenv('SWFED_PLAYER', 'none');
var_dump(swfed_player(), swfed_player_head());
echo swfed_swf_embed($src, $object), "\n";

echo "== a copy of your own (a path or an http(s) URL)\n";
setenv('SWFED_PLAYER_URL', '/player/flapp.js');
setenv('SWFED_PLAYER', 'flapp');
echo trim(swfed_player_head()), "\n";
setenv('SWFED_PLAYER_URL', 'https://example.org/r/flapp.js');
echo trim(swfed_player_head()), "\n";

echo "== refused values are logged\n";
foreach (array('javascript:alert(1)', 'data:text/javascript,alert(1)') as $url) {
    setenv('SWFED_PLAYER_URL', $url);
    var_dump(swfed_player_head());
}
setenv('SWFED_PLAYER_URL', null);
setenv('SWFED_PLAYER', 'bogus');
var_dump(swfed_player());
$log = file_get_contents("$dir.log");
echo "logged: ", substr_count($log, 'SWFED_PLAYER_URL must be'), " ", substr_count($log, 'unknown SWFED_PLAYER'), "\n";

echo "== escaped in the attributes\n";
setenv('SWFED_PLAYER', 'flapp');
setenv('SWFED_PLAYER_URL', '/p.js"><script>alert(1)</script>');
$head = swfed_player_head();
echo has($head, '<script>alert'), " ", has($head, '&quot;&gt;&lt;script&gt;'), "\n";
echo swfed_swf_embed('./a.swf?x="><b>', $object), "\n";

setenv('SWFED_PLAYER', null);
setenv('SWFED_PLAYER_URL', null);
setenv('SWFED_TMP_DIR', null);
@rmdir($dir);
@unlink("$dir.log");
?>
===DONE===
--EXPECT--
== flapp (default)
flapp: module=yes pinned to a commit=yes
<flapp-player src="./swf.php?id=1&amp;shape_id=2"></flapp-player>
== flapp (not case sensitive)
flapp
== none
string(4) "none"
string(0) ""
<object>for Flash Player</object>
== a copy of your own (a path or an http(s) URL)
<script type="module" src="/player/flapp.js"></script>
<script type="module" src="https://example.org/r/flapp.js"></script>
== refused values are logged
string(0) ""
string(0) ""
string(4) "none"
logged: 2 1
== escaped in the attributes
no yes
<flapp-player src="./a.swf?x=&quot;&gt;&lt;b&gt;"></flapp-player>
===DONE===
