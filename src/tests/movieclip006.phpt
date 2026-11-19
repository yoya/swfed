--TEST--
replaceMovieClip() when the target DefineSprite is the first tag
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

// DefineSprite(id=1){ShowFrame, End}, PlaceObject2(depth=1, id=1, name="mc")
$sprite = pack('vv', 1, 1) . tag(1, '') . tag(0, '');
$place = "\x22" . pack('vv', 1, 1) . "mc\x00";
$swfdata = swf(6, tag(39, $sprite) . tag(26, $place) . tag(1, '') . tag(0, ''));

$obj = new SWFEditor();
$obj->input($swfdata);
var_dump($obj->replaceMovieClip('mc', file_get_contents('tests/saitama.swf')));
$codes = array_map(function ($t) { return $t['code']; }, $obj->getTagList());
echo implode(',', $codes), "\n";

$obj2 = new SWFEditor();
var_dump($obj2->input($obj->output()));
var_dump(count($obj2->getTagList()) === count($codes));
?>
===DONE===
--EXPECTF--
bool(true)
%d,%s,39,26,1,0
bool(true)
bool(true)
===DONE===
