--TEST--
replaceEditString() replaces every DefineEditText with the variable name
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
function edit($id) {
    // CharacterID, Bounds(nbits=0), flags=HasText, VariableName, InitialText
    return tag(37, pack('v', $id) . "\x00" . "\x80\x00" . "msg\0" . "hello\0");
}

$obj = new SWFEditor();
$obj->input(swf(6, edit(2) . edit(3) . tag(1, '') . tag(0, '')));
var_dump($obj->replaceEditString('msg', 'world'));
$out = $obj->output();
var_dump(substr_count($out, "hello\0"));
var_dump(substr_count($out, "world\0"));
var_dump($obj->replaceEditString('nope', 'x'));
?>
===DONE===
--EXPECTF--
bool(true)
int(0)
int(2)
bool(false)
===DONE===
