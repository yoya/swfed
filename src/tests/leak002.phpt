--TEST--
isShapeTagData() / isBitmapTagData() don't leak the tag they parse
--SKIPIF--
<?php
// getrusage() has no ru_maxrss on Windows
if (PHP_OS_FAMILY === 'Windows') {
    die('skip no ru_maxrss on Windows');
}
// RSS growth means nothing without the Zend allocator (run-tests.php -m / valgrind)
if (getenv('USE_ZEND_ALLOC') === '0') {
    die('skip RSS based check, not under valgrind');
}
?>
--FILE--
<?php

function rss() {
    $r = getrusage();
    // ru_maxrss is bytes on macOS, kilobytes on Linux
    return (PHP_OS_FAMILY === 'Darwin') ? $r['ru_maxrss'] : $r['ru_maxrss'] * 1024;
}

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
$shape = null;
$bitmap = null;
foreach ($obj->getTagList() as $seqno => $tagInfo) {
    if (($shape === null) && ($tagInfo['code'] == 2)) {   // DefineShape
        $shape = $obj->getTagData($seqno);
    }
    if (($bitmap === null) && ($tagInfo['code'] == 6)) {  // DefineBits
        $bitmap = $obj->getTagData($seqno);
    }
}

// each call used to leave the parsed tag (and a copy of its data) behind
$before = rss();
for ($i = 0; $i < 100000; $i++) {
    $r = $obj->isShapeTagData($shape);
}
$grow = (rss() - $before) / 1048576;
echo var_export($r, true), " ", ($grow < 4) ? "shape ok" : sprintf("shape grew %.1fMB", $grow), "\n";

$before = rss();
for ($i = 0; $i < 20000; $i++) {
    $r = $obj->isBitmapTagData($bitmap);
}
$grow = (rss() - $before) / 1048576;
echo var_export($r, true), " ", ($grow < 4) ? "bitmap ok" : sprintf("bitmap grew %.1fMB", $grow), "\n";

// the results are still right
var_dump($obj->isShapeTagData($shape), $obj->isShapeTagData($bitmap));
var_dump($obj->isBitmapTagData($bitmap), $obj->isBitmapTagData($shape));
?>
===DONE===
--EXPECT--
true shape ok
true bitmap ok
bool(true)
bool(false)
bool(true)
bool(false)
===DONE===
