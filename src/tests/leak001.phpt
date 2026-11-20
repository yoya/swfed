--TEST--
replaceMovieClip() / replaceShapeData() failures don't leak, and keep the original shape
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

// The failing calls print a C stderr message every time, so run them in
// a child process with stderr discarded.
$child = <<<'CODE'
function rss() {
    $r = getrusage();
    // ru_maxrss is bytes on macOS, kilobytes on Linux
    return (PHP_OS_FAMILY === 'Darwin') ? $r['ru_maxrss'] : $r['ru_maxrss'] * 1024;
}
$obj = new SWFEditor();
$obj->input(file_get_contents('tests/negimiku.swf'));
$before = rss();
for ($i = 0; $i < 200000; $i++) {
    $r = $obj->replaceMovieClip('miku', str_repeat('X', 4096));
}
$grow = (rss() - $before) / 1048576;
echo var_export($r, true), " ", ($grow < 4) ? "movieclip ok" : sprintf("movieclip grew %.1fMB", $grow), "\n";

$obj->input(file_get_contents('tests/saitama.swf'));
$shape = $obj->getShapeData(2);
$bad = pack('vV', (2 << 6) | 0x3f, 2000) . "\x02\x00" . str_repeat("\xff", 1998);
$before = rss();
for ($i = 0; $i < 20000; $i++) {
    $r = $obj->replaceShapeData(2, $bad);
}
$grow = (rss() - $before) / 1048576;
echo var_export($r, true), " ", ($grow < 8) ? "shape ok" : sprintf("shape grew %.1fMB", $grow), "\n";

// old-format fallback that fails must keep the original shape
$r = $obj->replaceShapeData(2, str_repeat("\xff", 100));
echo var_export($r, true), " ", ($obj->getShapeData(2) === $shape) ? "original kept" : "original lost", "\n";
CODE;

$cmd = escapeshellarg(PHP_BINARY) . ' -n'
     . ' -d ' . escapeshellarg('extension_dir=' . ini_get('extension_dir'))
     . ' -d extension=swfed'
     . ' -r ' . escapeshellarg($child);
$p = proc_open($cmd, array(1 => array('pipe', 'w'), 2 => array('file', '/dev/null', 'w')), $pipes);
echo stream_get_contents($pipes[1]);
fclose($pipes[1]);
proc_close($p);
?>
===DONE===
--EXPECT--
false movieclip ok
false shape ok
false original kept
===DONE===
