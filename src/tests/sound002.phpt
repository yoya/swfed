--TEST--
replaceTagDataByCID() keeps the CID of DefineSound
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
function sound_swf($id, $payload) {
    $flags = (2 << 4) | (3 << 2) | (1 << 1) | 1;
    $sound = pack('vCV', $id, $flags, 4) . $payload;
    return swf(6, tag(9, "\xff\xff\xff") . tag(14, $sound) . tag(1, '') . tag(0, ''));
}

$obj = new SWFEditor();
$obj->input(sound_swf(1, "\x00\x00AAAA"));
$src = new SWFEditor();
$src->input(sound_swf(5, "\x00\x00BBBB"));

var_dump($obj->replaceTagDataByCID(1, $src->getTagData(1)));
$d = array_values($obj->getTagDetail(1));
var_dump($d[0]); // sound_id
var_dump($obj->getSoundData(1));
var_dump($obj->getSoundData(5));
?>
===DONE===
--EXPECTF--
bool(true)
int(1)
string(6) "%0%0BBBB"
%Abool(false)
===DONE===
