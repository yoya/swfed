--TEST--
output() fails for a shape whose NewStyles can't be written, instead of writing a broken shape (#180)
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
// DefineShape2: one solid fill, then a StyleChangeRecord with NewStyles that has $n solid fill styles.
// More than 32767 can be read, but not written: NumFillBits is 4 bits, i.e. up to 15 bits per index
function shape($n) {
    return tag(22, pack('v', 2) . "\x00" . "\x01" . "\x00\xff\x00\x00" . "\x00" . "\x10"
                   . "\x40"                                                   // StyleChangeRecord: StateNewStyles
                   . "\xff" . pack('v', $n) . str_repeat("\x00\xff\x00\x00", $n) . "\x00" // the new styles
                   . "\x40" . "\x00");                                        // NumFillBits 4, then EndShapeRecord
}

foreach (array(32767, 40000) as $n) {
    $o = new SWFEditor();
    $o->input(swf(6, shape($n) . tag(26, "\x02" . pack('vv', 1, 2)) . tag(1, '') . tag(0, '')));
    $d = $o->getTagDetail(0);
    $rebuild = $o->rebuild();
    $out = $o->output();
    echo "$n fill styles: detail=", is_array($d) ? 'array' : var_export($d, true), " rebuild=", var_export($rebuild, true);
    if (is_string($out)) {
        $o2 = new SWFEditor();
        $o2->input($out);
        echo " output can be read back: ", var_export(is_array($o2->getTagDetail(0)), true), "\n";
    } else {
        echo " output=false (", (strpos((string) SWFEditor::getLastError(), 'too many styles') !== false) ? 'too many styles' : 'other', ")",
             " getTagData=", var_export($o->getTagData(0), true), "\n";
    }
}
?>
===DONE===
--EXPECT--
32767 fill styles: detail=array rebuild=true output can be read back: true
40000 fill styles: detail=array rebuild=true output=false (too many styles) getTagData=false
===DONE===
