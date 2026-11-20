--TEST--
DefineButton2 with several records, BlendMode and FilterList survives rebuild()
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
// MATRIX with translate only; it does not end on a byte boundary
function matrix($x, $y) {
    $b = new Bits();
    $b->ub(1, 0); $b->ub(1, 0); $b->ub(5, 10); $b->sb(10, $x); $b->sb(10, $y);
    return $b->bytes();
}
$cxform = "\x00";
$blur = "\x01" . pack('VV', 0x40000, 0x40000) . "\x08"; // Blur: BlurX/Y=4.0, 1 pass
$records = "\x0f" . pack('vv', 1, 1) . matrix(100, 200) . $cxform
         . "\x31" . pack('vv', 2, 2) . matrix(-30, 40) . $cxform . "\x01" . $blur . "\x05"
         . "\x00";
$button = tag(34, pack('v', 3) . "\x00" . pack('v', 0) . $records);
$swf = swf(8, tag(9, "\xff\xff\xff") . $button . tag(1, '') . tag(0, ''));

$o = new SWFEditor();
$o->input($swf);
var_dump($o->rebuild());
$n1 = $o->output();
$o2 = new SWFEditor();
$o2->input($n1);
$o2->rebuild();
var_dump($o2->output() === $n1);
$o2->swfInfo();
exit(0);
?>
--EXPECTF--
bool(true)
bool(true)
%Ahas_blend_mode=0 has_filter_list=0
%wstate_hittest=1 state_down=1 state_over=1 state_up=1
%wcharacter_id=1 place_depth=1
%Afilter_list_len=0 blend_mode=0
%whas_blend_mode=1 has_filter_list=1
%wstate_hittest=0 state_down=0 state_over=0 state_up=1
%wcharacter_id=2 place_depth=2
%Afilter_list_len=11 blend_mode=5
%A
