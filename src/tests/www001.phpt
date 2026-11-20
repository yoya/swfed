--TEST--
www/define.php: the script tag of Ruffle that the demo pages include
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

function ruffle_tag($url) {
    if ($url === null) {
        putenv('SWFED_RUFFLE_URL'); // unset
    } else {
        putenv("SWFED_RUFFLE_URL=$url");
    }
    return swfed_ruffle_script_tag();
}
function has($tag, $needle) {
    return (strpos($tag, $needle) !== false) ? 'yes' : 'no';
}

// default: the pinned version on unpkg, checked with Subresource Integrity, played without a click
$tag = ruffle_tag(null);
echo "default: pinned=", has($tag, 'src="https://unpkg.com/@ruffle-rs/ruffle@0.8.0/ruffle.js"'),
     " integrity=", has($tag, 'integrity="sha384-'), " crossorigin=", has($tag, 'crossorigin="anonymous"'),
     " autoplay=", has($tag, 'autoplay: "on"'), "\n";

// an empty value: no emulator (a browser with Flash Player)
var_dump(ruffle_tag(''));

// a copy of your own: a path or an http(s) URL, without the integrity of the default
foreach (array('/ruffle/ruffle.js', './ruffle.js', 'https://example.org/r/ruffle.js') as $url) {
    $tag = ruffle_tag($url);
    echo $url, ": src=", has($tag, 'src="' . $url . '"'), " integrity=", has($tag, 'integrity'), "\n";
}

// anything else is refused (and logged)
var_dump(ruffle_tag('javascript:alert(1)'));
var_dump(ruffle_tag('data:text/javascript,alert(1)'));
echo "logged: ", (substr_count(file_get_contents("$dir.log"), 'SWFED_RUFFLE_URL must be') == 2) ? 'twice' : 'no', "\n";

// the URL is escaped in the attribute
$tag = ruffle_tag('/r.js"><script>alert(1)</script>');
echo "escaped: ", has($tag, '<script>alert') , " ", has($tag, '&quot;&gt;&lt;script&gt;'), "\n";

putenv('SWFED_RUFFLE_URL');
putenv('SWFED_TMP_DIR');
@rmdir($dir);
@unlink("$dir.log");
?>
===DONE===
--EXPECT--
default: pinned=yes integrity=yes crossorigin=yes autoplay=yes
string(0) ""
/ruffle/ruffle.js: src=yes integrity=no
./ruffle.js: src=yes integrity=no
https://example.org/r/ruffle.js: src=yes integrity=no
string(0) ""
string(0) ""
logged: twice
escaped: no yes
===DONE===
