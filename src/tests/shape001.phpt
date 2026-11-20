--TEST--
NewStyles style bit counts come from each new style array, not a running total
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
    public $bits = '';
    public $pos = 0;
    function ub($n, $v) {
        if ($n > 0) {
            $this->bits .= str_pad(decbin($v & ((1 << $n) - 1)), $n, '0', STR_PAD_LEFT);
        }
    }
    function sb($n, $v) { $this->ub($n, ($v < 0) ? ((1 << $n) + $v) : $v); }
    function raw($bytes) { // byte aligned data
        $this->bits .= str_repeat('0', (8 - strlen($this->bits) % 8) % 8);
        foreach (str_split($bytes) as $c) {
            $this->bits .= str_pad(decbin(ord($c)), 8, '0', STR_PAD_LEFT);
        }
    }
    function bytes() {
        $b = $this->bits . str_repeat('0', (8 - strlen($this->bits) % 8) % 8);
        $out = '';
        foreach (str_split($b, 8) as $c) {
            $out .= chr(bindec($c));
        }
        return $out;
    }
    function rub($n) { $v = $n ? bindec(substr($this->bits, $this->pos, $n)) : 0; $this->pos += $n; return $v; }
    function align() { $this->pos = (int) ceil($this->pos / 8) * 8; }
}
// DefineShape2 with one fill, then $n records that each switch to a new
// style array holding one fill (NumFillBits needs only 1 bit)
function shape($n) {
    $b = new Bits();
    $b->ub(4, 1); $b->ub(4, 0);                       // NumFillBits=1, NumLineBits=0
    for ($i = 0; $i < $n; $i++) {
        $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 1);
        $b->ub(5, 10); $b->sb(10, $i * 10); $b->sb(10, 0);  // MoveTo
        $b->ub(1, 1);                                 // FillStyle0
        $b->raw("\x01" . "\x00" . chr($i) . "\x00\x00" . "\x00"); // 1 solid fill, 0 line
        $b->ub(4, 1); $b->ub(4, 0);
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 5); $b->ub(1, 0); $b->ub(1, 1); $b->sb(7, 10);
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 5); $b->ub(1, 0); $b->ub(1, 0); $b->sb(7, 10);
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 5); $b->ub(1, 1); $b->sb(7, -10); $b->sb(7, -10);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(22, pack('v', 1) . "\x00" . "\x01\x00\xff\x00\x00" . "\x00" . $b->bytes());
}
// NumFillBits written after each NewStyles of the DefineShape2 above
function newstyles_fill_bits($data) {
    $b = new Bits();
    $b->raw(substr($data, ((ord($data[0]) & 0x3f) == 0x3f) ? 6 : 2));
    $b->rub(16);                                   // ShapeId
    $n = $b->rub(5); $b->rub(4 * $n); $b->align(); // RECT
    $b->rub(8 + 8 + 24 + 8);                       // 1 solid fill, 0 line
    $fill = $b->rub(4); $line = $b->rub(4);
    $out = array();
    while (true) {
        if ($b->rub(1) == 0) {                     // StyleChange or End
            $f = $b->rub(5);
            if ($f == 0) {
                break;
            }
            if ($f & 1) { $n = $b->rub(5); $b->rub(2 * $n); }
            if ($f & 2) { $b->rub($fill); }
            if ($f & 4) { $b->rub($fill); }
            if ($f & 8) { $b->rub($line); }
            if ($f & 16) {
                $b->align();
                $count = $b->rub(8); $b->rub(32 * $count);  // solid RGB fills
                $count = $b->rub(8); $b->rub(40 * $count);  // lines
                $fill = $b->rub(4); $line = $b->rub(4);
                $out[] = $fill;
            }
        } else {                                   // edge
            $straight = $b->rub(1);
            $n = $b->rub(4) + 2;
            if ($straight) {
                if ($b->rub(1)) { $b->rub(2 * $n); } else { $b->rub(1); $b->rub($n); }
            } else {
                $b->rub(4 * $n);
            }
        }
    }
    return implode(',', array_unique($out));
}

$o = new SWFEditor();
foreach (array(1, 40) as $n) {
    $o->input(swf(6, tag(9, "\xff\xff\xff") . shape($n) . tag(1, '') . tag(0, '')));
    var_dump($o->rebuild());
    $o2 = new SWFEditor();
    $o2->input($o->output());
    echo "n=$n: NumFillBits=", newstyles_fill_bits($o2->getTagData(1)),
         " parse=", is_array($o2->getTagDetail(1)) ? "ok" : "broken", "\n";
}
?>
===DONE===
--EXPECT--
bool(true)
n=1: NumFillBits=1 parse=ok
bool(true)
n=40: NumFillBits=1 parse=ok
===DONE===
