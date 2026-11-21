--TEST--
A broken tag in a DefineSprite fails the sprite instead of dropping the tags after it (#152)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

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
function sprite($inner) {
    return tag(39, pack('vv', 1, 1) . $inner);
}
$place = tag(26, "\x02" . pack('vv', 1, 2));
$too_long = "\xbf\x06" . pack('V', 65535) . "junk";   // PlaceObject2 that claims 65535 bytes

$cases = array(
    'well formed'              => sprite($place . tag(1, '') . tag(0, '')),
    'broken tag'               => sprite($place . tag(1, '') . $too_long),
    'broken tag after End'     => sprite($place . tag(1, '') . tag(0, '') . $too_long),
    'half a tag header at end' => sprite($place . tag(1, '') . "\x40"),
    'no End tag'               => sprite($place . tag(1, '')),
);
foreach ($cases as $name => $sprite) {
    $o = new SWFEditor();
    $in = $o->input(swf(6, $sprite . tag(1, '') . tag(0, '')));
    $detail = $o->getTagDetail(0);
    $rebuild = $o->rebuild();
    $out = $o->getTagData(0);
    echo "$name: input=", var_export($in, true),
         " detail=", is_array($detail) ? 'array' : var_export($detail, true),
         " rebuild=", var_export($rebuild, true),
         " tag=", ($out === $sprite) ? 'as it was' : (strlen($sprite) . '=>' . strlen($out) . ' bytes'), "\n";
}
?>
===DONE===
--EXPECT--
well formed: input=true detail=array rebuild=true tag=as it was
broken tag: input=true detail=false rebuild=false tag=as it was
broken tag after End: input=true detail=array rebuild=true tag=27=>17 bytes
half a tag header at end: input=true detail=false rebuild=false tag=as it was
no End tag: input=true detail=array rebuild=true tag=as it was
===DONE===
