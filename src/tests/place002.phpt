--TEST--
swfInfo() shows the ClipDepth of a PlaceObject2 as clip_depth, and the ratio as ratio (#159)
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
// PlaceObject2: HasClipDepth, HasRatio, HasCharacter: Depth 3, CharacterId 1, Ratio 7, ClipDepth 5
$place = tag(26, "\x52" . pack('vvvv', 3, 1, 7, 5));
$o = new SWFEditor();
var_dump($o->input(swf(6, $place . tag(1, '') . tag(0, ''))));
ob_start();
$o->swfInfo();
$info = ob_get_clean();
preg_match_all('/^\s*(ratio|clip_depth)=\d+$/m', $info, $m);
echo implode("\n", array_map('trim', $m[0])), "\n";
// and both are written back as they were
var_dump($o->rebuild(), $o->getTagData(0) === $place);
?>
===DONE===
--EXPECT--
bool(true)
ratio=7
clip_depth=5
bool(true)
bool(true)
===DONE===
