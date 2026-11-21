--TEST--
replaceActionStrings(): a failure in a sprite is reported even when other action tags follow (#151)
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
function push($s) {
    $d = "\x00" . $s . "\0";
    return "\x96" . pack('v', strlen($d)) . $d;
}

$broken = "\x96\x05\x00\x00ab";   // Push whose length runs past the end of the tag
$sprite_broken = tag(39, pack('vv', 1, 1) . tag(12, $broken) . tag(1, '') . tag(0, ''));
$sprite_good   = tag(39, pack('vv', 2, 1) . tag(12, push('a') . "\x00") . tag(1, '') . tag(0, ''));
$action = tag(12, push('a') . "\x00");
$place = tag(26, "\x02" . pack('vv', 1, 2));

$cases = array(
    'broken sprite only'                => $sprite_broken,
    'broken sprite, then DoAction'      => $sprite_broken . $action,
    'broken sprite, then PlaceObject2'  => $sprite_broken . $sprite_good . $place,
    'broken sprite, then a good sprite' => $sprite_broken . $sprite_good,
    'good sprite, then DoAction'        => $sprite_good . $action,
);
foreach ($cases as $name => $tags) {
    $o = new SWFEditor();
    $o->input(swf(6, $tags . tag(1, '') . tag(0, '')));
    $r = $o->replaceActionStrings(array('a' => 'b'));
    echo "$name: ", var_export($r, true), "\n";
}
?>
===DONE===
--EXPECT--
broken sprite only: false
broken sprite, then DoAction: false
broken sprite, then PlaceObject2: false
broken sprite, then a good sprite: false
good sprite, then DoAction: true
===DONE===
