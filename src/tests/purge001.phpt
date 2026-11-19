--TEST--
purgeUselessContents() keeps contents referenced by PlaceObject3 / ExportAssets / SymbolClass
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
class Bits {
    private $bits = '';
    function ub($n, $v) {
        if ($n > 0) {
            $this->bits .= str_pad(decbin($v & ((1 << $n) - 1)), $n, '0', STR_PAD_LEFT);
        }
    }
    function sb($n, $v) { $this->ub($n, ($v < 0) ? ((1 << $n) + $v) : $v); }
    function bytes() {
        $b = $this->bits . str_repeat('0', (8 - strlen($this->bits) % 8) % 8);
        $out = '';
        foreach (str_split($b, 8) as $c) {
            $out .= chr(bindec($c));
        }
        return $out;
    }
}
function lossless($code, $id) {
    return tag($code, pack('vCvv', $id, 5, 1, 1) . gzcompress("\xff\xff\x00\x00"));
}
// DefineShape(id) filled with bitmap
function shape($id, $bitmap) {
    $b = new Bits();
    $b->ub(4, 1); $b->ub(4, 0);                    // NumFillBits, NumLineBits
    $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 1);
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0);      // MoveTo(0,0)
    $b->ub(1, 1);                                  // FillStyle0
    foreach (array(array(400, 0), array(0, 400), array(-400, 0), array(0, -400)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 10); $b->ub(1, 1); $b->sb(12, $d[0]); $b->sb(12, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(2, pack('v', $id) . "\x00" . "\x01\x41" . pack('v', $bitmap) . "\x00" . "\x00" . $b->bytes());
}
function sprite($id, $child) {
    return tag(39, pack('vv', $id, 1) . tag(26, "\x02" . pack('vv', 1, $child)) . tag(1, '') . tag(0, ''));
}
function names($o) {
    return implode(',', array_map(function ($t) {
        return isset($t['tagName']) ? $t['tagName'] : '(' . $t['code'] . ')';
    }, $o->getTagList()));
}

$attrs = tag(69, pack('V', 0));
$bg = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
$cases = array(
    'PlaceObject3' => swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1)
                             . tag(70, "\x02\x00" . pack('vv', 1, 2)) . $end),
    'ExportAssets' => swf(6, $bg . lossless(20, 1) . shape(2, 1) . sprite(3, 2) . lossless(20, 4)
                             . tag(56, pack('vv', 2, 3) . "mc_linkage\0" . pack('v', 4) . "bmp_linkage\0") . $end),
    'SymbolClass'  => swf(9, $attrs . $bg . lossless(20, 1) . shape(2, 1) . sprite(3, 2) . lossless(36, 4)
                             . tag(76, pack('vv', 2, 3) . "Mc\0" . pack('v', 4) . "Bmp\0") . $end),
    // DefineBitsLossless(5) and DefineShape(6) are not referenced: still purged
    'unused'       => swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1) . lossless(20, 5) . shape(6, 5)
                             . tag(70, "\x02\x00" . pack('vv', 1, 2)) . $end),
);
foreach ($cases as $name => $swf) {
    $o = new SWFEditor();
    $o->input($swf);
    $o->purgeUselessContents();
    echo $name, ": ", names($o), "\n";
}
?>
===DONE===
--EXPECT--
PlaceObject3: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,(70),ShowFrame,End
ExportAssets: SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,DefineBitsLossless,Export,ShowFrame,End
SymbolClass: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,DefineBitsLossless2,(76),ShowFrame,End
unused: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,(70),ShowFrame,End
===DONE===
