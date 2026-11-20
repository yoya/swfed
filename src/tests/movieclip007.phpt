--TEST--
replaceMovieClip() rejects nested DefineSprite in the inserted SWF
--SKIPIF--
<?php
// getrusage() has no ru_maxrss on Windows
if (PHP_OS_FAMILY === 'Windows') {
    die('skip no ru_maxrss on Windows');
}
?>
--FILE--
<?php

function tag($code, $body) {
    $len = strlen($body);
    if ($len < 0x3f) {
        return pack('v', ($code << 6) | $len) . $body;
    }
    return pack('vV', ($code << 6) | 0x3f, $len) . $body;
}
function swf($version, $tags) {
    $body = "\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr($version) . pack('V', 8 + strlen($body)) . $body;
}
function rss() {
    $r = getrusage();
    // ru_maxrss is bytes on macOS, kilobytes on Linux
    return (PHP_OS_FAMILY === 'Darwin') ? $r['ru_maxrss'] : $r['ru_maxrss'] * 1024;
}

// DefineSprite in DefineSprite, 4000 levels
$s = tag(1, '') . tag(0, '');
for ($i = 4000; $i > 0; $i--) {
    $s = tag(39, pack('vv', $i, 1) . $s) . tag(0, '');
}
$child = swf(6, $s . tag(1, ''));

$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
$before_swf = $o->output();
$before = rss();
var_dump($o->replaceMovieClip('miku', $child));
$grow = (rss() - $before) / 1048576;
echo ($grow < 20) ? "ok\n" : sprintf("grew %.1fMB\n", $grow);
var_dump($o->output() === $before_swf);

// a single level DefineSprite is still fine
$child = swf(6, tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, '')) . tag(1, '') . tag(0, ''));
var_dump($o->replaceMovieClip('miku', $child));
?>
===DONE===
--EXPECTF--
%Abool(false)
ok
bool(true)
bool(true)
===DONE===
