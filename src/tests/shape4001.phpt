--TEST--
DefineShape4 / DefineMorphShape2 are handled as shapes (round trip, purge, replaceMovieClip)
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
    $b->ub(1, 0); $b->ub(1, 0);
    $b->ub(1, $with_styles ? 1 : 0); $b->ub(1, 0); $b->ub(1, $with_styles ? 1 : 0);
    $b->ub(1, 1); // MoveTo
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0);
    if ($with_styles) {
        $b->ub(1, 1); // FillStyle0
        $b->ub(1, 1); // LineStyle
    }
    foreach (array(array($size, 0), array(0, $size), array(-$size, 0), array(0, -$size)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 12); $b->ub(1, 1);
        $b->sb(14, $d[0]); $b->sb(14, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return $b->bytes();
}
function lossless2($id) {
    return tag(36, pack('vCvv', $id, 5, 1, 1) . gzcompress("\xff\xff\x00\x00"));
}
// DefineShape4: bitmap fill (clipped, id=$bitmap) + LINESTYLE2, flags = non-scaling strokes
function shape4($id, $bitmap) {
    $fills = "\x01" . "\x41" . pack('v', $bitmap) . "\x00";
    $lines = "\x01" . pack('v', 20) . "\x00\x00" . "\x00\x00\x00\xff";
    return tag(83, pack('v', $id) . rect(0, 2000, 0, 2000) . rect(10, 1990, 10, 1990)
                   . "\x02" . $fills . $lines . square_edges(2000, true));
}
// DefineMorphShape2: solid fill + MORPHLINESTYLE2
function morph2($id) {
    $fills = "\x01" . "\x00" . "\xff\x00\x00\xff" . "\x00\x00\xff\xff";
    $lines = "\x01" . pack('vv', 20, 40) . "\x00\x00" . "\x00\x00\x00\xff" . "\x00\x00\x00\xff";
    $start = square_edges(2000, true);
    return tag(84, pack('v', $id) . rect(0, 2000, 0, 2000) . rect(0, 4000, 0, 4000)
                   . rect(10, 1990, 10, 1990) . rect(20, 3980, 20, 3980) . "\x01"
                   . pack('V', strlen($fills . $lines . $start))
                   . $fills . $lines . $start . square_edges(4000, false));
}
function codes($o) {
    return implode(',', array_map(function ($t) { return $t['code']; }, $o->getTagList()));
}

$place = tag(26, "\x02" . pack('vv', 1, 2));  // HasCharacter, depth=1 id=2
$swf4 = swf(8, tag(9, "\xff\xff\xff") . lossless2(1) . shape4(2, 1) . $place . tag(1, '') . tag(0, ''));
$swfm = swf(8, tag(9, "\xff\xff\xff") . morph2(2) . $place . tag(1, '') . tag(0, ''));

// byte after the RECTs (the flags) of the shape tag body
function shape_flags($o, $code, $nrect) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == $code) {
            $d = $o->getTagData($i);
            $pos = ((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2;
            $pos += 2; // character id
            for ($r = 0; $r < $nrect; $r++) {
                $nbits = ord($d[$pos]) >> 3;
                $pos += (int) ceil((5 + 4 * $nbits) / 8);
            }
            return sprintf('0x%02x', ord($d[$pos]));
        }
    }
}
// rebuild() parses and re-encodes the shape: the result is stable and keeps the flags
foreach (array('DefineShape4' => array($swf4, 83, 2), 'DefineMorphShape2' => array($swfm, 84, 4)) as $name => $c) {
    $o = new SWFEditor();
    $o->input($c[0]);
    var_dump($o->rebuild());
    $n1 = $o->output();
    $o2 = new SWFEditor();
    $o2->input($n1);
    $o2->rebuild();
    echo $name, ": stable=", ($o2->output() === $n1) ? "yes" : "no",
         " flags=", shape_flags($o, $c[1], $c[2]), "\n";
}

// purge keeps the bitmap used by DefineShape4
$o = new SWFEditor();
$o->input($swf4);
$o->purgeUselessContents();
echo codes($o), "\n";
$d = $o->getTagDetail(2);
echo "shape_id=", $d['shape_id'], " bitmap_ref=", implode(',', $d['bitmap_ref']), "\n";
var_dump(strlen($o->getShapeData(2)) > 0);

// replaceMovieClip renumbers the bitmap reference of DefineShape4
$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->replaceMovieClip('miku', $swf4));
$bitmap_id = $shape_ref = null;
foreach ($o->getTagList() as $i => $t) {
    if ($t['code'] == 36) {
        $bitmap_id = $o->getTagDetail($i)['image_id'];
    } elseif ($t['code'] == 83) {
        $shape_ref = $o->getTagDetail($i)['bitmap_ref'][0];
    }
}
var_dump($bitmap_id > 7, $bitmap_id === $shape_ref);
?>
===DONE===
--EXPECT--
bool(true)
DefineShape4: stable=yes flags=0x02
bool(true)
DefineMorphShape2: stable=yes flags=0x01
9,36,83,26,1,0
shape_id=2 bitmap_ref=1
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
