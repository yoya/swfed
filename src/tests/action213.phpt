--TEST--
setActionVariables() / replaceActionStrings(): a NUL in a string ends it, the bytes after it are not written (#145)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function tag($code, $body) {
    return pack('vV', ($code << 6) | 0x3f, strlen($body)) . $body;
}
function swf($tags) {
    $body = "\x78\x00\x05\x5f\x00\x00\x0f\xa0\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr(6) . pack('V', 8 + strlen($body)) . $body;
}
function push($s) {
    $d = "\x00" . $s . "\0";
    return "\x96" . pack('v', strlen($d)) . $d;
}
function disasm($o) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 12) {
            $o->disasmActionData($o->getActionData($i));
        }
    }
}

// what an application passes on as the value of "name": a string, then
// "\0" + type 0 (String) + "1", which used to become a second pushed value
$evil = "isAdmin\0" . "\x00" . "1";

echo "== setActionVariables: value\n";
$o = new SWFEditor();
$o->input(swf(tag(1, '') . tag(0, '')));
var_dump($o->setActionVariables(array('name' => $evil)));
disasm($o);

echo "== setActionVariables: key\n";
$o = new SWFEditor();
$o->input(swf(tag(1, '') . tag(0, '')));
var_dump($o->setActionVariables(array("k\0\x00x" => 'v', 'plain' => "a\0")));
disasm($o);

echo "== replaceActionStrings: Push, ConstantPool, GetURL\n";
$pool = "\x88" . pack('v', 2 + strlen("PH\0second\0")) . pack('v', 2) . "PH\0second\0";
$geturl = "\x83" . pack('v', strlen("PH\0_self\0")) . "PH\0_self\0";
$o = new SWFEditor();
$o->input(swf(tag(12, $pool . push('name') . push('PH') . "\x1d" . $geturl . "\x00") . tag(1, '') . tag(0, '')));
var_dump($o->replaceActionStrings(array('PH' => $evil)));
disasm($o);
// the result is still a well formed action list
$o2 = new SWFEditor();
var_dump($o2->input($o->output()), $o2->rebuild());

echo "== a key with a NUL matches nothing\n";
$o = new SWFEditor();
$o->input(swf(tag(12, push('PH') . "\x00") . tag(1, '') . tag(0, '')));
var_dump($o->replaceActionStrings(array("PH\0x" => 'changed')));
disasm($o);
?>
===DONE===
--EXPECT--
== setActionVariables: value
bool(true)
    Push Data(String)name
    Push Data(String)isAdmin
    Set Variable
    End
== setActionVariables: key
bool(true)
    Push Data(String)k
    Push Data(String)v
    Set Variable
    Push Data(String)plain
    Push Data(String)a
    Set Variable
    End
== replaceActionStrings: Push, ConstantPool, GetURL
bool(true)
    Declare Dictionary:
    		[0]'isAdmin'
		[1]'second'

    Push Data(String)name
    Push Data(String)isAdmin
    Set Variable
    Get URL (String)isAdmin (String)_self
    End
bool(true)
bool(true)
== a key with a NUL matches nothing
bool(true)
    Push Data(String)PH
    End
===DONE===
