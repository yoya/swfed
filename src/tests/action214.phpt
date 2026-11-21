--TEST--
A replaceActionStrings() that fails leaves the actions as they were (#150)
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
function jump($offset) {
    return "\x99\x02\x00" . pack('v', $offset);
}

// Jump over Push "a"; Push "target"; Push of a string that can't take 600 more bytes
$long = str_repeat('b', 65000);
$code = jump(strlen(push('a'))) . push('a') . push('target') . push($long) . "\x00";
$map = array('a' => 'AAAAAAAAAA', $long => str_repeat('c', 65600));

foreach (array('DoAction' => tag(12, $code), 'DoAction in a sprite' => tag(39, pack('vv', 1, 1) . tag(12, $code) . tag(1, '') . tag(0, ''))) as $where => $t) {
    echo "== $where\n";
    $o = new SWFEditor();
    $o->input(swf(6, $t . tag(1, '') . tag(0, '')));
    $tag_before = $o->getTagData(0);
    var_dump($o->replaceActionStrings($map));
    echo "error: ", (strpos((string) SWFEditor::getLastError(), 'too long') !== false) ? 'too long' : 'other', "\n";
    echo "tag kept: ", var_export($o->getTagData(0) === $tag_before, true), "\n";
    // an edit that succeeds after the failure writes the actions that were there
    var_dump($o->replaceActionStrings(array('target' => 'TARGET-TARGET')));
    $tag_after = $o->getTagData(0);
    echo "jump still over Push \"a\": ", var_export(strpos($tag_after, jump(strlen(push('a'))) . push('a') . push('TARGET-TARGET')) !== false, true), "\n";
    echo "nothing of the failed call: ", var_export(strpos($tag_after, 'AAAAAAAAAA') === false, true), "\n";
    $o2 = new SWFEditor();
    var_dump($o2->input($o->output()), $o2->rebuild());
}

echo "== DoAction, then setActionVariables\n";
$o = new SWFEditor();
$o->input(swf(6, tag(12, $code) . tag(1, '') . tag(0, '')));
$actions_before = $o->getActionData(0);
var_dump($o->replaceActionStrings($map));
echo "actions kept: ", var_export($o->getActionData(0) === $actions_before, true), "\n";
var_dump($o->setActionVariables(array('v' => '1')));
$actions = $o->getActionData(0);
echo "variables + the same actions: ", var_export(substr($actions, -strlen($actions_before)) === $actions_before, true), "\n";

echo "== a replacement that succeeds still moves the jump\n";
$o = new SWFEditor();
$o->input(swf(6, tag(12, jump(strlen(push('a'))) . push('a') . push('target') . "\x00") . tag(1, '') . tag(0, '')));
var_dump($o->replaceActionStrings(array('a' => 'AAAAAAAAAA')));
echo var_export($o->getActionData(0) === jump(strlen(push('AAAAAAAAAA'))) . push('AAAAAAAAAA') . push('target') . "\x00", true), "\n";
?>
===DONE===
--EXPECT--
== DoAction
bool(false)
error: too long
tag kept: true
bool(true)
jump still over Push "a": true
nothing of the failed call: true
bool(true)
bool(true)
== DoAction in a sprite
bool(false)
error: too long
tag kept: true
bool(true)
jump still over Push "a": true
nothing of the failed call: true
bool(true)
bool(true)
== DoAction, then setActionVariables
bool(false)
actions kept: true
bool(true)
variables + the same actions: true
== a replacement that succeeds still moves the jump
bool(true)
true
===DONE===
