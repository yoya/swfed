--TEST--
DefineShape4 / DefineMorphShape2: bitmaps used by fills and LINESTYLE2 fills, and the shape adjustment of replaceBitmapData()
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
// returns array(array(xmin, xmax, ymin, ymax), position after the rect)
function read_rect($d, $pos) {
    $nbits = ord($d[$pos]) >> 3;
    $len = (int) ceil((5 + 4 * $nbits) / 8);
    $bits = '';
    for ($i = 0; $i < $len; $i++) {
        $bits .= str_pad(decbin(ord($d[$pos + $i])), 8, '0', STR_PAD_LEFT);
    }
    $vals = array();
    for ($k = 0, $p = 5; $k < 4; $k++, $p += $nbits) {
        $v = bindec(substr($bits, $p, $nbits));
        $vals[] = ($v >= (1 << ($nbits - 1))) ? $v - (1 << $nbits) : $v;
    }
    return array($vals, $pos + $len);
}
// a square of $size twips; fill style 1 / line style 1 are used if asked for
function square_edges($size, $fill, $line) {
    $b = new Bits();
    $b->ub(4, $fill ? 1 : 0); // NumFillBits
    $b->ub(4, $line ? 1 : 0); // NumLineBits
    $b->ub(1, 0); $b->ub(1, 0);
    $b->ub(1, $fill ? 1 : 0); $b->ub(1, 0); $b->ub(1, $line ? 1 : 0);
    $b->ub(1, 1); // MoveTo
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0);
    if ($fill) { $b->ub(1, 1); } // FillStyle0
    if ($line) { $b->ub(1, 1); } // LineStyle
    foreach (array(array($size, 0), array(0, $size), array(-$size, 0), array(0, -$size)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 12); $b->ub(1, 1);
        $b->sb(14, $d[0]); $b->sb(14, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return $b->bytes();
}
function bitmap100($id) { // DefineBitsLossless2 100x100
    return tag(36, pack('vCvv', $id, 5, 100, 100) . gzcompress(str_repeat("\xff\xff\x00\x00", 10000)));
}
function names($o) {
    return implode(',', array_map(function ($t) { return $t['code']; }, $o->getTagList()));
}

$solid = "\xff\x00\x00\xff"; // RGBA
$bg = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
$place = tag(26, "\x02" . pack('vv', 1, 2));
$bmpfill = function ($id) { return "\x41" . pack('v', $id) . "\x00"; }; // clipped bitmap fill, identity matrix
$morphbmpfill = function ($id) { return "\x41" . pack('v', $id) . "\x00" . "\x00"; }; // start / end matrix
// DefineShape4 (83): the bitmap fill is in FILLSTYLE (fill=true) or in the LINESTYLE2 (fill=false)
function shape4($id, $bitmap, $in_fill, $bmpfill, $solid) {
    $line_head = "\x01" . pack('v', 20);
    if ($in_fill) {
        $fills = "\x01" . $bmpfill($bitmap);
        $lines = $line_head . "\x00\x00" . $solid;
    } else {
        $fills = "\x01\x00" . $solid;
        $lines = $line_head . "\x08\x00" . $bmpfill($bitmap); // HasFillFlag
    }
    return tag(83, pack('v', $id) . rect(0, 2000, 0, 2000) . rect(10, 1990, 10, 1990)
                   . "\x02" . $fills . $lines . square_edges(2000, true, true));
}
// DefineMorphShape2 (84)
function morph2($id, $bitmap, $in_fill, $morphbmpfill, $solid) {
    $line_head = "\x01" . pack('vv', 20, 40);
    if ($in_fill) {
        $fills = "\x01" . $morphbmpfill($bitmap);
        $lines = $line_head . "\x00\x00" . $solid . $solid;
    } else {
        $fills = "\x01\x00" . $solid . $solid;
        $lines = $line_head . "\x08\x00" . $morphbmpfill($bitmap);
    }
    $start = square_edges(2000, true, true);
    return tag(84, pack('v', $id) . rect(0, 2000, 0, 2000) . rect(0, 4000, 0, 4000)
                   . rect(10, 1990, 10, 1990) . rect(20, 3980, 20, 3980) . "\x01"
                   . pack('V', strlen($fills . $lines . $start))
                   . $fills . $lines . $start . square_edges(4000, false, false));
}

echo "== bitmaps used by DefineShape4 / DefineMorphShape2\n";
$cases = array(
    'DefineShape4, fill'              => shape4(2, 1, true, $bmpfill, $solid),
    'DefineShape4, LINESTYLE2 fill'   => shape4(2, 1, false, $bmpfill, $solid),
    'DefineMorphShape2, fill'         => morph2(2, 1, true, $morphbmpfill, $solid),
    'DefineMorphShape2, line fill'    => morph2(2, 1, false, $morphbmpfill, $solid),
);
foreach ($cases as $name => $shape) {
    $s = swf(8, $bg . bitmap100(1) . $shape . $place . $end);
    $o = new SWFEditor();
    $o->input($s);
    $d = $o->getTagDetail(2);
    echo $name, ": bitmap_ref=", isset($d['bitmap_ref']) ? implode(',', $d['bitmap_ref']) : '-',
         " shapes_using_1=", implode(',', $o->getShapeIdListByBitmapRef(1)), "\n";
    // the bitmap is only used by the shape: purge keeps it
    $o->purgeUselessContents();
    echo "  purge: ", names($o), "\n";
    // replaceMovieClip() renumbers the bitmap and the reference follows
    $o->input(file_get_contents('tests/negimiku.swf'));
    var_dump($o->replaceMovieClip('miku', $s));
    $bitmap_id = $ref = null;
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 36) {
            $bitmap_id = $o->getTagDetail($i)['image_id'];
        } elseif (($t['code'] == 83) || ($t['code'] == 84)) {
            $ref = $o->getTagDetail($i)['bitmap_ref'][0];
        }
    }
    var_dump($bitmap_id > 7, $bitmap_id === $ref);
}

echo "== shape adjustment of replaceBitmapData() (100x100 -> 400x300)\n";
$jpeg = file_get_contents('tests/5.jpg');
// DefineShape3 with the same content, for comparison: without line styles the
// styles and the edges after the bounds are encoded in the same way
$shape3 = tag(32, pack('v', 2) . rect(0, 2000, 0, 2000) . "\x01" . $bmpfill(1) . "\x00"
                  . square_edges(2000, true, false));
$shape4 = tag(83, pack('v', 2) . rect(0, 2000, 0, 2000) . rect(10, 1990, 10, 1990)
                  . "\x02" . "\x01" . $bmpfill(1) . "\x00" . square_edges(2000, true, false));
function shape_body($o) { // the body of the shape (id=2) tag
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 32 || $t['code'] == 83) {
            $d = $o->getTagData($i);
            return substr($d, ((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2);
        }
    }
}
function adjusted($tag, $mode, $bg, $place, $end) {
    global $jpeg;
    $o = new SWFEditor();
    $o->input(swf(8, $bg . bitmap100(1) . $tag . $place . $end));
    $o->setShapeAdjustMode($mode);
    var_dump($o->replaceBitmapData(1, $jpeg));
    return shape_body($o);
}
$modes = array(
    'MATRIX_RESCALE' => SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE,
    'RECT_RESIZE'    => SWFEditor::SHAPE_BITMAP_RECT_RESIZE,
    'TYPE_TILLED'    => SWFEditor::SHAPE_BITMAP_TYPE_TILLED,
);
$none = adjusted($shape4, SWFEditor::SHAPE_BITMAP_NONE, $bg, $place, $end);
foreach ($modes as $name => $mode) {
    $b3 = adjusted($shape3, $mode, $bg, $place, $end);
    $b4 = adjusted($shape4, $mode, $bg, $place, $end);
    list($r3, $p3) = read_rect($b3, 2);
    list($r4, $p4) = read_rect($b4, 2);
    list($e4, $p4e) = read_rect($b4, $p4);
    $t3 = substr($b3, $p3);              // after the bounds
    $t4 = substr($b4, $p4e + 1);         // after the bounds, EdgeBounds and the flags
    echo $name, ": changed=", ($b4 !== $none) ? "yes" : "no",
         " same as DefineShape3=", (($r3 === $r4) && ($t3 === $t4)) ? "yes" : "no", "\n";
    if ($name == 'RECT_RESIZE') {
        echo "  bounds=", implode(',', $r4), " EdgeBounds=", implode(',', $e4), " flags=", ord($b4[$p4e]), "\n";
    }
    if ($name == 'TYPE_TILLED') {
        echo "  fill type=", sprintf('0x%02x', ord($t4[1])), "\n"; // FillStyleCount, then the type
    }
}

echo "== the fill of LINESTYLE2 follows the same adjustment\n";
// both fills use bitmap 1 with the same identity matrix: after the adjustment their matrices are the same
$shape4l = tag(83, pack('v', 2) . rect(0, 2000, 0, 2000) . rect(10, 1990, 10, 1990)
                   . "\x02" . "\x01" . $bmpfill(1)
                   . "\x01" . pack('v', 20) . "\x08\x00" . $bmpfill(1)
                   . square_edges(2000, true, true));
foreach ($modes as $name => $mode) {
    $b = adjusted($shape4l, $mode, $bg, $place, $end);
    list(, $p) = read_rect($b, 2);                               // bounds
    list(, $p) = read_rect($b, $p);                              // EdgeBounds
    $pf = $p + 1 + 1;                                            // flags, FillStyleCount: the fill's type
    $pl = strpos($b, "\x01\x14\x00\x08\x00", $pf);               // LineStyleCount, width=20, flags (HasFillFlag)
    $pl_type = $pl + 5;                                          // the type of the fill of LINESTYLE2
    $line_type = ord($b[$pl_type]);
    $fill_type = ord($b[$pf]);
    if ($name == 'MATRIX_RESCALE') {
        $len = $pl - ($pf + 3);                                  // the matrix of the fill: after type and bitmap id
        $mf = substr($b, $pf + 3, $len);
        $ml = substr($b, $pl_type + 3, $len);
        echo $name, ": has scale=", (ord($mf[0]) & 0x80) ? "yes" : "no",
             " same matrix=", ($mf === $ml) ? "yes" : "no", " matrix length=", $len, "\n";
    } elseif ($name == 'TYPE_TILLED') {
        echo $name, ": fill=", sprintf('0x%02x', $fill_type), " line fill=", sprintf('0x%02x', $line_type), "\n";
    } else {
        echo $name, ": line fill unchanged=", (ord($b[$pl_type]) == 0x41) ? "yes" : "no", "\n";
    }
}

echo "== DefineMorphShape2 is not adjusted\n";
$morph = morph2(2, 1, true, $morphbmpfill, $solid);
$o = new SWFEditor();
$o->input(swf(8, $bg . bitmap100(1) . $morph . $place . $end));
$o->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
$before = $o->getTagData(2);
var_dump($o->replaceBitmapData(1, $jpeg));
var_dump($o->getTagData(2) === $before);
?>
===DONE===
--EXPECTF--
== bitmaps used by DefineShape4 / DefineMorphShape2
DefineShape4, fill: bitmap_ref=1 shapes_using_1=2
  purge: 9,36,83,26,1,0
bool(true)
bool(true)
bool(true)
DefineShape4, LINESTYLE2 fill: bitmap_ref=1 shapes_using_1=2
  purge: 9,36,83,26,1,0
bool(true)
bool(true)
bool(true)
DefineMorphShape2, fill: bitmap_ref=1 shapes_using_1=2
  purge: 9,36,84,26,1,0
bool(true)
bool(true)
bool(true)
DefineMorphShape2, line fill: bitmap_ref=1 shapes_using_1=2
  purge: 9,36,84,26,1,0
bool(true)
bool(true)
bool(true)
== shape adjustment of replaceBitmapData() (100x100 -> 400x300)
bool(true)
bool(true)
bool(true)
MATRIX_RESCALE: changed=yes same as DefineShape3=yes
bool(true)
bool(true)
RECT_RESIZE: changed=yes same as DefineShape3=yes
  bounds=0,8000,0,6000 EdgeBounds=40,7960,30,5970 flags=2
bool(true)
bool(true)
TYPE_TILLED: changed=yes same as DefineShape3=yes
  fill type=0x40
== the fill of LINESTYLE2 follows the same adjustment
bool(true)
MATRIX_RESCALE: has scale=yes same matrix=yes matrix length=6
bool(true)
RECT_RESIZE: line fill unchanged=yes
bool(true)
TYPE_TILLED: fill=0x40 line fill=0x40
== DefineMorphShape2 is not adjusted
%Abool(true)
bool(true)
===DONE===
