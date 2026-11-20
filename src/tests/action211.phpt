--TEST--
replaceActionStrings() also replaces button actions and onClipEvent actions
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
function act($id, $data = null) {
    return ($id < 0x80) ? chr($id) : chr($id) . pack('v', strlen($data)) . $data;
}
$trace = act(0x96, "\x00LONGSTRING\0") . act(0x26) . act(0x00);

// DefineButton2: one record, one condaction (on release)
$records2 = "\x01" . pack('vv', 9, 1) . "\x00" . "\x00" . "\x00";
$button2 = tag(34, pack('v', 3) . "\x00" . pack('v', 2 + strlen($records2)) . $records2
                   . pack('v', 0) . "\x08\x00" . $trace);
// DefineButton: one record, actions
$button1 = tag(7, pack('v', 4) . "\x01" . pack('vv', 9, 1) . "\x00" . "\x00" . $trace);
// PlaceObject2 with onClipEvent: SWF 6+ uses UI32 event flags
function clip_actions($flag_size) {
    global $trace;
    $f = function ($v) use ($flag_size) { return ($flag_size == 4) ? pack('V', $v) : pack('v', $v); };
    $load = $f(0x01);                        // onClipEvent(load)
    $out = pack('v', 0) . $f(0x01 | ($flag_size == 4 ? 0x020000 : 0))
         . $load . pack('V', strlen($trace)) . $trace;
    if ($flag_size == 4) {                   // on(keyPress "<Space>"): KeyCode precedes actions
        $out .= $f(0x020000) . pack('V', 1 + strlen($trace)) . "\x20" . $trace;
    }
    return $out . $f(0);
}
$sprite = tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, ''));

foreach (array(6 => 4, 5 => 2) as $version => $flag_size) {
    $place = tag(26, "\x82" . pack('vv', 1, 1) . clip_actions($flag_size));
    $swf = swf($version, tag(9, "\xff\xff\xff") . $button2 . $button1 . $sprite . $place
                         . tag(1, '') . tag(0, ''));
    $o = new SWFEditor();
    $o->input($swf);
    var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
    $out = $o->output();
    echo "SWF $version: LONGSTRING=", substr_count($out, 'LONGSTRING'),
         " S=", substr_count($out, "\x00S\x00"), "\n";
    $o2 = new SWFEditor();
    var_dump($o2->input($out));
    var_dump($o2->rebuild());
    var_dump($o2->output() === $out);
}
?>
===DONE===
--EXPECT--
bool(true)
SWF 6: LONGSTRING=0 S=4
bool(true)
bool(true)
bool(true)
bool(true)
SWF 5: LONGSTRING=0 S=3
bool(true)
bool(true)
bool(true)
===DONE===
