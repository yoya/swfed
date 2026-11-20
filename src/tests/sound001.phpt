--TEST--
getTagDetail() returns unsigned DefineSound rate / flags
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
function sound_swf($rate) {
    // SoundFormat=2 (MP3), SoundRate=$rate, 16bit, stereo
    $flags = (2 << 4) | ($rate << 2) | (1 << 1) | 1;
    $sound = pack('vCV', 1, $flags, 4) . "\x00\x00\xff\xfb";
    return swf(6, tag(9, "\xff\xff\xff") . tag(14, $sound) . tag(1, '') . tag(0, ''));
}

$obj = new SWFEditor();
foreach (array(0, 1, 2, 3) as $rate) {
    $obj->input(sound_swf($rate));
    $d = $obj->getTagDetail(1);
    echo "rate=", $d['rate'], " is_16bits=", $d['is_16bits'],
         " is_stereo=", $d['is_stereo'], "\n";
}
?>
===DONE===
--EXPECT--
rate=0 is_16bits=1 is_stereo=1
rate=1 is_16bits=1 is_stereo=1
rate=2 is_16bits=1 is_stereo=1
rate=3 is_16bits=1 is_stereo=1
===DONE===
