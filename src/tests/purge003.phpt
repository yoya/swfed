--TEST--
purgeUselessContents() removes nothing when a tag in use can't be parsed (#179)
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
function lossless($id) {
    return tag(20, pack('vCvv', $id, 5, 1, 1) . gzcompress("\x00\xff\x00\x00"));
}
function place($depth, $id) { // PlaceObject2: HasCharacter
    return tag(26, "\x02" . pack('vv', $depth, $id));
}
// DefineShape2 with a clipped bitmap fill of $bitmap and no edges
function shape($id, $bitmap) {
    return tag(22, pack('v', $id) . "\x00" . "\x01" . "\x41" . pack('v', $bitmap) . "\x00" . "\x00" . "\x10" . "\x00");
}
// the same, with a second fill style of an unknown type: it can't be parsed
function broken_shape($id, $bitmap) {
    return tag(22, pack('v', $id) . "\x00" . "\x02" . "\x41" . pack('v', $bitmap) . "\x00" . "\x77" . "\x00" . "\x10" . "\x00");
}
function sprite($id, $inner) {
    return tag(39, pack('vv', $id, 1) . $inner);
}
$too_long = "\xbf\x06" . pack('V', 65535) . "junk";   // a tag that claims 65535 bytes
// DefineButton2 with one record (character $cid) and no action; broken: the record is cut off
function button($id, $cid) {
    return tag(34, pack('v', $id) . "\x00" . pack('v', 0) . "\x01" . pack('vv', $cid, 1) . "\x00" . "\x00" . "\x00");
}
function broken_button($id, $cid) {
    return tag(34, pack('v', $id) . "\x00" . pack('v', 0) . "\x01" . pack('v', $cid));
}

$end = tag(1, '') . tag(0, '');
$cases = array(
    'nothing broken'                      => lossless(1) . lossless(9) . shape(2, 1) . shape(8, 9) . place(1, 2) . $end,
    'broken shape in use'                 => lossless(1) . lossless(9) . broken_shape(2, 1) . place(1, 2) . $end,
    'broken shape not in use'             => lossless(1) . lossless(9) . shape(2, 1) . broken_shape(8, 9) . place(1, 2) . $end,
    'broken sprite in use'                => lossless(1) . shape(2, 1) . sprite(3, place(1, 2) . tag(1, '') . $too_long) . place(1, 3) . $end,
    'broken sprite not in use'            => lossless(1) . shape(2, 1) . sprite(3, place(1, 2) . tag(1, '') . $too_long) . $end,
    'broken PlaceObject2'                 => lossless(1) . shape(2, 1) . tag(26, "\x06" . pack('vv', 1, 2)) . $end,   // HasMatrix, but no matrix
    'broken PlaceObject2 in a sprite'     => lossless(1) . shape(2, 1) . sprite(3, tag(26, "\x06" . pack('vv', 1, 2)) . tag(1, '') . tag(0, '')) . place(1, 3) . $end,
    'button in use'                       => lossless(1) . shape(2, 1) . button(4, 2) . place(1, 4) . $end,
    'broken button in use'                => lossless(1) . shape(2, 1) . broken_button(4, 2) . place(1, 4) . $end,
);
foreach ($cases as $name => $tags) {
    $o = new SWFEditor();
    $o->input(swf(8, $tags));
    $before = array_column($o->getTagList(), 'code');
    $r = $o->purgeUselessContents();
    $error = (strpos((string) SWFEditor::getLastError(), 'nothing is removed') !== false) ? ' (nothing is removed)' : '';
    $after = array_column($o->getTagList(), 'code');
    echo "$name: ", var_export($r, true), $error, " ", implode(' ', $before), ($after === $before) ? ' => unchanged' : (' => ' . implode(' ', $after)), "\n";
}
?>
===DONE===
--EXPECT--
nothing broken: true 20 20 22 22 26 1 0 => 20 22 26 1 0
broken shape in use: false (nothing is removed) 20 20 22 26 1 0 => unchanged
broken shape not in use: true 20 20 22 22 26 1 0 => 20 22 26 1 0
broken sprite in use: false (nothing is removed) 20 22 39 26 1 0 => unchanged
broken sprite not in use: true 20 22 39 1 0 => 39 1 0
broken PlaceObject2: false (nothing is removed) 20 22 26 1 0 => unchanged
broken PlaceObject2 in a sprite: false (nothing is removed) 20 22 39 26 1 0 => unchanged
button in use: true 20 22 34 26 1 0 => unchanged
broken button in use: false (nothing is removed) 20 22 34 26 1 0 => unchanged
===DONE===
