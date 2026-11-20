--TEST--
applyShapeMatrixFactor() / MATRIX_RESCALE scale fill matrices correctly
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
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
    private $pos = 0;
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
function matrix($scale, $rotate) {
    $b = new Bits();
    $b->ub(1, $scale ? 1 : 0);
    if ($scale) { $b->ub(5, 24); $b->sb(24, $scale[0]); $b->sb(24, $scale[1]); }
    $b->ub(1, $rotate ? 1 : 0);
    if ($rotate) { $b->ub(5, 24); $b->sb(24, $rotate[0]); $b->sb(24, $rotate[1]); }
    $b->ub(5, 0);
    return $b->bytes();
}
function shape($id, $bitmap, $matrix) {
    $b = new Bits();
    $b->ub(4, 1); $b->ub(4, 0);
    $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 1);
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0); $b->ub(1, 1);
    foreach (array(array(400, 0), array(0, 400), array(-400, 0), array(0, -400)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 10); $b->ub(1, 1); $b->sb(12, $d[0]); $b->sb(12, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(2, pack('v', $id) . "\x00" . "\x01\x41" . pack('v', $bitmap) . $matrix . "\x00" . $b->bytes());
}
// ScaleX RotateSkew0 RotateSkew1 ScaleY of the first fill of the DefineShape
function fill_matrix($o) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 2) {
            $d = $o->getTagData($i);
            $b = Bits::from(substr($d, ((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2));
            $b->rub(16);                                   // ShapeId
            $n = $b->rub(5); $b->rub(4 * $n); $b->align(); // RECT
            $b->rub(8 + 8 + 16);                           // count, type, BitmapId
            $sx = $sy = 65536; $r0 = $r1 = 0;
            if ($b->rub(1)) { $n = $b->rub(5); $sx = $b->rsb($n); $sy = $b->rsb($n); }
            if ($b->rub(1)) { $n = $b->rub(5); $r0 = $b->rsb($n); $r1 = $b->rsb($n); }
            return "$sx $r0 $r1 $sy";
        }
    }
}
function chunk($type, $data) {
    return pack('N', strlen($data)) . $type . $data . pack('N', crc32($type . $data));
}
$lossless = tag(20, pack('vCvv', 1, 5, 1, 1) . gzcompress("\x00\xff\x00\x00"));
$png2x1 = "\x89PNG\r\n\x1a\n" . chunk('IHDR', pack('NNCCCCC', 2, 1, 8, 2, 0, 0, 0))
        . chunk('IDAT', gzcompress("\x00" . "\xff\x00\x00\x00\xff\x00")) . chunk('IEND', '');
$r90 = array(20 * 65536, -20 * 65536);   // 90 degree rotation, 20 twips per pixel

$o = new SWFEditor();
// scale omitted = 1.0
$o->input(swf(6, $lossless . shape(2, 1, matrix(null, null)) . tag(1, '') . tag(0, '')));
$o->applyShapeMatrixFactor(2, 2.0, 3.0, 0, 0, 0);
echo "no scale:  ", fill_matrix($o), "\n";
// rotated: x' = RotateSkew1 * y, y' = RotateSkew0 * x
$o->input(swf(6, $lossless . shape(2, 1, matrix(array(0, 0), $r90)) . tag(1, '') . tag(0, '')));
$o->applyShapeMatrixFactor(2, 2.0, 3.0, 0, 0, 0);
echo "rotated:   ", fill_matrix($o), "\n";
// replacing the 1x1 bitmap with a 2x1 one halves the bitmap x scale (a column)
$o->input(swf(6, $lossless . shape(2, 1, matrix(array(0, 0), $r90)) . tag(1, '') . tag(0, '')));
$o->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
var_dump($o->replaceBitmapData(1, $png2x1));
echo "rescale:   ", fill_matrix($o), "\n";
?>
===DONE===
--EXPECT--
no scale:  131072 0 0 196608
rotated:   0 3932160 -2621440 0
bool(true)
rescale:   0 655360 -1310720 0
===DONE===
