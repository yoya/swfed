--TEST--
DefineMorphShape: getTagDetail() style counts, applyShape*Factor() report unsupported
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
function rect($xmin, $xmax, $ymin, $ymax) {
    $b = new Bits();
    $b->ub(5, 16);
    foreach (array($xmin, $xmax, $ymin, $ymax) as $v) {
        $b->sb(16, $v);
    }
    return $b->bytes();
}
function square_edges($size, $with_styles) {
    $b = new Bits();
    $b->ub(4, $with_styles ? 1 : 0); // NumFillBits
    $b->ub(4, $with_styles ? 1 : 0); // NumLineBits
    // StyleChangeRecord: NewStyles=0 LineStyle FillStyle1=0 FillStyle0 MoveTo=1
    $b->ub(1, 0);
    $b->ub(1, 0);
    $b->ub(1, $with_styles ? 1 : 0);
    $b->ub(1, 0);
    $b->ub(1, $with_styles ? 1 : 0);
    $b->ub(1, 1);
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0);
    if ($with_styles) {
        $b->ub(1, 1); // FillStyle0
        $b->ub(1, 1); // LineStyle
    }
    foreach (array(array($size, 0), array(0, $size),
                   array(-$size, 0), array(0, -$size)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 14 - 2); $b->ub(1, 1);
        $b->sb(14, $d[0]); $b->sb(14, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0); // EndShapeRecord
    return $b->bytes();
}

$fills = "\x01" . "\x00" . "\xff\x00\x00\xff" . "\x00\x00\xff\xff";
$lines = "\x01" . pack('vv', 20, 40) . "\x00\x00\x00\xff" . "\x00\x00\x00\xff";
$start = square_edges(2000, true);
$end = square_edges(4000, false);
$morph = pack('v', 1) . rect(0, 2000, 0, 2000) . rect(0, 4000, 0, 4000)
       . pack('V', strlen($fills . $lines . $start))
       . $fills . $lines . $start . $end;

$obj = new SWFEditor();
$obj->input(swf(6, tag(9, "\xff\xff\xff") . tag(46, $morph) . tag(1, '') . tag(0, '')));
$before = $obj->output();
$d = $obj->getTagDetail(1);
echo $d['shape_id'], " ", $d['fill_styles.count'], " ", $d['line_styles.count'], "\n";
var_dump($obj->applyShapeMatrixFactor(1, 2.0, 2.0, 0, 0, 0));
var_dump($obj->applyShapeRectFactor(1, 2.0, 2.0, 0, 0));
var_dump($obj->output() === $before);
?>
===DONE===
--EXPECTF--
1 1 1
%Abool(false)
%Abool(false)
bool(true)
===DONE===
