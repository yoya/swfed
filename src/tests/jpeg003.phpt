--TEST--
swfInfo() does not read a truncated SOF segment
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

// DefineBitsJPEG2(id=1): SOI + SOF0 with segment length 2 (no body)
$jpeg = "\xff\xd8" . "\xff\xc0\x00\x02";
$obj = new SWFEditor();
$obj->input(swf(6, tag(21, pack('v', 1) . $jpeg) . tag(1, '') . tag(0, '')));
var_dump($obj->getBitmapSize(1));
$obj->swfInfo();
exit(0);
?>
--EXPECTF--
bool(false)
%A    image_id=1  jpeg_data_size=6
        SOI(0xD8): len=0
        SOF0(0xC0): len=0
%A
