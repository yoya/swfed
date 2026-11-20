--TEST--
applyShapeRectFactor() scales curves too, around the rect origin, and refuses edges that don't fit
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
    function bytes() {
        $b = $this->bits . str_repeat('0', (8 - strlen($this->bits) % 8) % 8);
        $out = '';
        foreach (str_split($b, 8) as $c) {
            $out .= chr(bindec($c));
        }
        return $out;
    }
    static function from($data) {
        $b = new Bits();
        foreach (str_split($data) as $c) {
            $b->bits .= str_pad(decbin(ord($c)), 8, '0', STR_PAD_LEFT);
        }
        return $b;
    }
    function rub($n) { $v = $n ? bindec(substr($this->bits, $this->pos, $n)) : 0; $this->pos += $n; return $v; }
    function rsb($n) { $v = $this->rub($n); return ($n && $v >= (1 << ($n - 1))) ? $v - (1 << $n) : $v; }
    function align() { $this->pos = (int) ceil($this->pos / 8) * 8; }
}
function rect($x0, $x1, $y0, $y1) {
    $b = new Bits();
    $b->ub(5, 20);
    foreach (array($x0, $x1, $y0, $y1) as $v) { $b->sb(20, $v); }
    return $b->bytes();
}
// (1000,1000)-(2000,2000): a line, a curve, a line, a line; one line style
function shape() {
    $b = new Bits();
    $b->ub(4, 0); $b->ub(4, 1);
    $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1);
    $b->ub(5, 14); $b->sb(14, 1000); $b->sb(14, 1000); $b->ub(1, 1);       // MoveTo, LineStyle
    $line = function ($dx, $dy) use ($b) { $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 11); $b->ub(1, 1); $b->sb(13, $dx); $b->sb(13, $dy); };
    $line(1000, 0);
    $b->ub(1, 1); $b->ub(1, 0); $b->ub(4, 11);                             // curve
    $b->sb(13, 0); $b->sb(13, 500); $b->sb(13, 0); $b->sb(13, 500);
    $line(-1000, 0);
    $line(0, -1000);
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(2, pack('v', 1) . rect(1000, 2000, 1000, 2000) . "\x00" . "\x01" . pack('v', 20) . "\x00\x00\x00"
                  . $b->bytes());
}
// rect of the DefineShape and the bounding box of every point of its records
function bounds($o) {
    $d = $o->getTagData(1);
    $b = Bits::from(substr($d, ((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2));
    $b->rub(16);
    $n = $b->rub(5);
    $rect = array($b->rsb($n), $b->rsb($n), $b->rsb($n), $b->rsb($n));
    $b->align();
    $b->rub(8);                     // no fill
    $b->rub(8 + 16 + 24);           // one line style
    $fill = $b->rub(4); $line = $b->rub(4);
    $x = $y = 0; $xs = $ys = array();
    while (true) {
        if ($b->rub(1) == 0) {
            $f = $b->rub(5);
            if ($f == 0) { break; }
            if ($f & 1) { $n = $b->rub(5); $x = $b->rsb($n); $y = $b->rsb($n); $xs[] = $x; $ys[] = $y; }
            if ($f & 2) { $b->rub($fill); }
            if ($f & 4) { $b->rub($fill); }
            if ($f & 8) { $b->rub($line); }
        } elseif ($b->rub(1)) {     // straight
            $n = $b->rub(4) + 2;
            if ($b->rub(1)) { $x += $b->rsb($n); $y += $b->rsb($n); }
            elseif ($b->rub(1)) { $y += $b->rsb($n); } else { $x += $b->rsb($n); }
            $xs[] = $x; $ys[] = $y;
        } else {                    // curved
            $n = $b->rub(4) + 2;
            $x += $b->rsb($n); $y += $b->rsb($n); $xs[] = $x; $ys[] = $y;
            $x += $b->rsb($n); $y += $b->rsb($n); $xs[] = $x; $ys[] = $y;
        }
    }
    return sprintf("rect=(%d,%d)-(%d,%d) points=(%d,%d)-(%d,%d)", $rect[0], $rect[2], $rect[1], $rect[3],
                   min($xs), min($ys), max($xs), max($ys));
}

$swf = swf(6, tag(9, "\xff\xff\xff") . shape() . tag(1, '') . tag(0, ''));
$o = new SWFEditor();
$o->input($swf);
echo bounds($o), "\n";
var_dump($o->applyShapeRectFactor(1, 2.0, 2.0, 0, 0));
echo bounds($o), "\n";

// a 1000 twips edge scaled 200 times needs more than 17 bits
$o->input($swf);
$before = $o->getTagData(1);
var_dump($o->applyShapeRectFactor(1, 200.0, 1.0, 0, 0));
var_dump($o->getTagData(1) === $before);
?>
===DONE===
--EXPECTF--
rect=(1000,1000)-(2000,2000) points=(1000,1000)-(2000,2000)
bool(true)
rect=(1000,1000)-(3000,3000) points=(1000,1000)-(3000,3000)
%Abool(false)
bool(true)
===DONE===
