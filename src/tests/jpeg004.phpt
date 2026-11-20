--TEST--
replaceJpegData() rejects non-JPEG data and ignores data after the EOI
--FILE--
<?php

function replaced($jpeg) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/saitama.swf'));
    $r = $o->replaceJpegData(1, $jpeg);
    $o2 = new SWFEditor();
    $o2->input($o->output());
    return array($r, $o2->getJpegData(1));
}

$prog = file_get_contents('tests/5-progressive.jpg');
$base = file_get_contents('tests/5.jpg');
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
$orig = $o->getJpegData(1);

list($r, $p) = replaced($prog);
var_dump($r);
// trailing data after the EOI (text, or another JPEG) is dropped
list($r, $j) = replaced($prog . "TRAILING DATA");
var_dump($r, $j === $p);
list($r, $j) = replaced($prog . $base);
var_dump($r, $j === $p);
// 0xFF fill bytes before a marker
list($r, $b) = replaced($base);
list($r, $j) = replaced("\xff\xd8\xff\xff" . substr($base, 3));
var_dump($r, $j === $b);
// not a JPEG / truncated JPEG: fail and keep the original image
list($r, $j) = replaced(file_get_contents('tests/negimiku001.png'));
var_dump($r, $j === $orig);
list($r, $j) = replaced(substr($base, 0, 600));
var_dump($r, $j === $orig);
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
%Abool(false)
bool(true)
%Abool(false)
bool(true)
===DONE===
