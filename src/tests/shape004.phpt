--TEST--
A shape whose edges move the pen out of the range of a MoveTo fails to parse, without an integer overflow (#181)
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
function bits($s) { // "0101..." to bytes
    $s .= str_repeat('0', (8 - strlen($s) % 8) % 8);
    return implode('', array_map(function ($b) { return chr(bindec($b)); }, str_split($s, 8)));
}
// DefineShape2: MoveTo($x, 0), then $n horizontal lines of $dx twips each (17 bit deltas)
function shape($x, $n, $dx) {
    $r = '0' . '00001' . sprintf('%05b', 31) . substr(sprintf('%032b', $x & 0xffffffff), 1) . sprintf('%031b', 0);
    $d = substr(sprintf('%032b', $dx & 0xffffffff), -17);
    for ($i = 0; $i < $n; $i++) {
        $r .= '1' . '1' . '1111' . '0' . '0' . $d;   // straight edge, NumBits 15+2, horizontal
    }
    $r .= '000000';
    return tag(22, pack('v', 2) . "\x00" . "\x01" . "\x00\xff\x00\x00" . "\x00" . "\x10" . bits($r));
}

$cases = array(
    'inside the range'            => array(0x3fff0000, 1, 65535),        // ends at 2^30 - 1
    'one twip over the range'     => array(0x3fff0000, 2, 32768),        // ends at 2^30
    'over the int range'          => array(0x3fffffff, 40000, 65535),
    'under the range'             => array(-0x40000000, 1, -1),
);
foreach ($cases as $name => $c) {
    $o = new SWFEditor();
    $o->input(swf(6, shape($c[0], $c[1], $c[2]) . tag(26, "\x02" . pack('vv', 1, 2)) . tag(1, '') . tag(0, '')));
    $tag = $o->getTagData(0);
    $d = $o->getTagDetail(0);
    $error = (strpos((string) SWFEditor::getLastError(), 'out of range') !== false) ? ' (out of range)' : '';
    $rebuild = $o->rebuild();
    if ($rebuild) { // re-encoded: read it back
        $o2 = new SWFEditor();
        $o2->input($o->output());
        $state = is_array($o2->getTagDetail(0)) ? 'reads back' : 'BROKEN';
    } else {
        $state = ($o->getTagData(0) === $tag) ? 'as it was' : 'CHANGED';
    }
    echo "$name: detail=", is_array($d) ? 'array' : var_export($d, true), $error,
         " rebuild=", var_export($rebuild, true), " tag $state\n";
}
?>
===DONE===
--EXPECT--
inside the range: detail=array rebuild=true tag reads back
one twip over the range: detail=false (out of range) rebuild=false tag as it was
over the int range: detail=false (out of range) rebuild=false tag as it was
under the range: detail=false (out of range) rebuild=false tag as it was
===DONE===
