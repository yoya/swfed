--TEST--
replaceMovieClip() renumbers every bitmap fill of a shape that uses several bitmaps
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
function lossless($id, $width) {
    return tag(20, pack('vCvv', $id, 5, $width, 1) . gzcompress(str_repeat("\x00\xff\x00\x00", $width)));
}
// DefineShape(2): left square filled with bitmap 1, right square with bitmap 8
function shape() {
    $b = new Bits();
    $b->ub(4, 2); $b->ub(4, 0);
    foreach (array(1 => 0, 2 => 400) as $fill => $x) {
        $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 1);
        $b->ub(5, 11); $b->sb(11, $x); $b->sb(11, 0); $b->ub(2, $fill);
        foreach (array(array(400, 0), array(0, 400), array(-400, 0), array(0, -400)) as $d) {
            $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 10); $b->ub(1, 1); $b->sb(12, $d[0]); $b->sb(12, $d[1]);
        }
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(2, pack('v', 2) . "\x00" . "\x02" . "\x41" . pack('v', 1) . "\x00" . "\x41" . pack('v', 8) . "\x00"
                  . "\x00" . $b->bytes());
}
$child = swf(6, lossless(1, 1) . lossless(8, 2) . shape()
                . tag(26, "\x02" . pack('vv', 1, 2)) . tag(1, '') . tag(0, ''));

$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));   // uses CID 1..7
var_dump($o->replaceMovieClip('miku', $child));
$by_width = array();
$refs = null;
foreach ($o->getTagList() as $i => $t) {
    if ($t['code'] == 20) {
        $d = $o->getTagDetail($i);
        $by_width[$d['width']] = $d['image_id'];
    }
}
foreach ($o->getTagList() as $i => $t) {
    if ($t['code'] == 2) {
        $d = $o->getTagDetail($i);
        if (isset($d['bitmap_ref']) && count($d['bitmap_ref']) == 2) {
            $refs = $d['bitmap_ref'];
        }
    }
}
// the 1px wide bitmap was id 1, the 2px wide one was id 8
echo "fill1 -> ", ($refs[0] === $by_width[1]) ? "bitmap 1" : "wrong", "\n";
echo "fill2 -> ", ($refs[1] === $by_width[2]) ? "bitmap 8" : "wrong", "\n";
var_dump($refs[0] !== $refs[1]);
?>
===DONE===
--EXPECT--
bool(true)
fill1 -> bitmap 1
fill2 -> bitmap 8
bool(true)
===DONE===
