--TEST--
DefineBitsLossless format=4 (15bit color) can be read back
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

// output of replaceBitmapData(..., rgb15) read back
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
var_dump($o->replaceBitmapData(1, file_get_contents('tests/negimiku001.png24'), '', array('rgb15' => true)));
$o2 = new SWFEditor();
$o2->input($o->output());
$png = $o2->getPNGData(1);
var_dump(substr($png, 1, 3));
$size = $o2->getBitmapSize(1);
echo $size['width'], "x", $size['height'], "\n";
// re-encoding the parsed 15bit pixels gives the same bytes
$o2->rebuild();
var_dump($o2->output() === $o->output());

// 3x1 format=4: red, green, white (row padded to 8 bytes)
$pixels = pack('n*', 0x7c00, 0x03e0, 0x7fff) . "\0\0";
$bits = pack('vCvv', 1, 4, 3, 1) . gzcompress($pixels);
$o->input(swf(6, tag(9, "\xff\xff\xff") . tag(20, $bits) . tag(1, '') . tag(0, '')));
var_dump(strlen($o->getPNGData(1)) > 0);
var_dump($o->replaceBitmapData(array('red' => 255, 'green' => 0, 'blue' => 0),
                               file_get_contents('tests/negimiku001.png')));
?>
===DONE===
--EXPECTF--
bool(true)
string(3) "PNG"
%dx%d
bool(true)
bool(true)
bool(true)
===DONE===
