--TEST--
SWFEditor frees its SWF data when the object is destroyed
--SKIPIF--
<?php
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
function work($swf) {
    $o = new SWFEditor();
    $o->input($swf);
    return strlen($o->output());
}

$swf = file_get_contents('tests/negimiku.swf');
work($swf);
$before = rss();
for ($i = 0; $i < 1400; $i++) {
    work($swf);
}
$grow = (rss() - $before) / 1048576;
echo ($grow < 20) ? "ok\n" : sprintf("grew %.1fMB\n", $grow);

// re-initializing would free data a running method may still use
$o = new SWFEditor();
try {
    $o->__construct();
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}

// a subclass that doesn't call the parent constructor is usable
class MyEditor extends SWFEditor {
    function __construct() {}
}
$m = new MyEditor();
var_dump($m->input($swf));
?>
===DONE===
--EXPECT--
ok
SWFEditor object is already initialized
bool(true)
===DONE===
