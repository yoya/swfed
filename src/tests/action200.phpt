--TEST--
swfInfo() shows Push Data Float / Double as IEEE754 values
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
function push($data) {
    return "\x96" . pack('v', strlen($data)) . $data;
}

// Float 1.5 = 0x3FC00000 (LE), Double -2.25 = 0xC002000000000000
// (SWF stores the upper 32bit word first, each word little endian)
$actions = push("\x01" . pack('V', 0x3FC00000))
         . push("\x06" . pack('V', 0xC0020000) . pack('V', 0))
         . "\x00";
$obj = new SWFEditor();
$obj->input(swf(6, tag(12, $actions) . tag(1, '') . tag(0, '')));
$obj->swfInfo();
exit(0);
?>
--EXPECTF--
%A(Float)1.500000%A(Double)-2.250000%A
