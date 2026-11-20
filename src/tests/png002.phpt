--TEST--
getPNGData() fails cleanly on palette index overflow and zero size bitmaps
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
function lossless($format, $width, $height, $extra, $raw) {
    return pack('vCvv', 1, $format, $width, $height) . $extra . gzcompress($raw);
}

// format=3, 4x1, 2 colors, but pixel index 5
$colormap = "\xff\x00\x00" . "\x00\xff\x00";
$bits = lossless(3, 4, 1, "\x01", $colormap . "\x00\x01\x05\x00");
$obj = new SWFEditor();
$obj->input(swf(6, tag(9, "\xff\xff\xff") . tag(20, $bits) . tag(1, '') . tag(0, '')));
var_dump($obj->getPNGData(1));

// format=5, width=0 (empty pixel data so it reaches the PNG encoder)
$bits = lossless(5, 0, 1, '', '');
$obj2 = new SWFEditor();
$obj2->input(swf(6, tag(9, "\xff\xff\xff") . tag(20, $bits) . tag(1, '') . tag(0, '')));
var_dump($obj2->getPNGData(1));
?>
===DONE===
--EXPECTF--
%Abool(false)
%Abool(false)
===DONE===
