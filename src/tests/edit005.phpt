--TEST--
getEditString() / replaceEditString(): only a name of decimal digits is tried as a character ID (#154)
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
// DefineEditText: empty bounds, HasText only
function edit($id, $variable, $text) {
    return tag(37, pack('v', $id) . "\x00" . "\x80\x00" . $variable . "\0" . $text . "\0");
}
function editor() {
    $o = new SWFEditor();
    $o->input(swf(6, edit(5, 'x', 'five') . edit(6, '5th', 'six') . edit(7, 'y', 'seven') . edit(12, '7', 'twelve')
                     . tag(1, '') . tag(0, '')));
    return $o;
}
function texts($o) {
    $r = array();
    foreach (array(5, 6, 7, 12) as $i => $id) {
        $d = $o->getTagDetail($i);
        $r[] = "$id=" . $d['initial_text'];
    }
    return implode(' ', $r);
}

$o = editor();
echo "== getEditString\n";
foreach (array('x', '5th', '5', '6', '007', '7', '12', '7up', ' 7', '7 ', '7.0', '+7', '0x7', 'nothing', '', '0', '65536', '100000') as $name) {
    echo var_export($name, true), " => ", var_export($o->getEditString($name), true), "\n";
}
echo "== replaceEditString\n";
foreach (array('5th', '5', '7', '7up', 'nothing', ' 12') as $name) {
    $o = editor();
    echo var_export($name, true), ": ", var_export($o->replaceEditString($name, 'NEW'), true), " ", texts($o), "\n";
}
?>
===DONE===
--EXPECT--
== getEditString
'x' => 'five'
'5th' => 'six'
'5' => 'five'
'6' => 'six'
'007' => 'seven'
'7' => 'seven'
'12' => 'twelve'
'7up' => false
' 7' => false
'7 ' => false
'7.0' => false
'+7' => false
'0x7' => false
'nothing' => false
'' => false
'0' => false
'65536' => false
'100000' => false
== replaceEditString
'5th': true 5=five 6=NEW 7=seven 12=twelve
'5': true 5=NEW 6=six 7=seven 12=twelve
'7': true 5=five 6=six 7=NEW 12=NEW
'7up': false 5=five 6=six 7=seven 12=twelve
'nothing': false 5=five 6=six 7=seven 12=twelve
' 12': false 5=five 6=six 7=seven 12=twelve
===DONE===
