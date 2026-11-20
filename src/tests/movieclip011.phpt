--TEST--
replaceMovieClip() follows the CID change in fonts, texts, buttons and DoInitAction
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
// [ [code, body], ... ] of a FWS
function tags($swf) {
    $nbits = ord($swf[8]) >> 3;
    return tags_in($swf, 8 + intdiv(5 + 4 * $nbits + 7, 8) + 4);
}
function tags_in($data, $pos) {
    $list = array();
    while ($pos + 2 <= strlen($data)) {
        $h = unpack('v', substr($data, $pos, 2))[1];
        $pos += 2;
        $len = $h & 0x3f;
        if ($len == 0x3f) {
            $len = unpack('V', substr($data, $pos, 4))[1];
            $pos += 4;
        }
        $list[] = array($h >> 6, substr($data, $pos, $len));
        $pos += $len;
        if (($h >> 6) == 0) {
            break;
        }
    }
    return $list;
}
function u16($s, $pos = 0) {
    return unpack('v', substr($s, $pos, 2))[1];
}
// tags added in front of the movie clip "miku" (DefineSprite id=7) of negimiku.swf
function added($tags) {
    $list = array();
    foreach (array_slice($tags, 9) as $t) {
        if (($t[0] == 39) && (u16($t[1]) == 7)) {
            break;
        }
        $list[] = $t;
    }
    return $list;
}
function replace($child, &$out = null) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf')); // CID 1-7
    $before = $o->output();
    $ret = $o->replaceMovieClip('miku', $child);
    var_dump($ret);
    $out = $o->output();
    if (! $ret) {
        var_dump($out === $before);
    }
    return tags($out);
}

$head = tag(69, pack('V', 0)) . tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
// DefineFont2 / DefineFont3 without glyphs
function font($code, $id) {
    return tag($code, pack('v', $id) . "\x00\x00\x01A" . pack('vv', 0, 2));
}
// DefineShape with a solid fill
function shape($id) {
    return tag(2, pack('v', $id) . "\x00" . "\x01\x00\xff\x00\x00" . "\x00" . "\x10" . "\x0a\x00");
}

echo "* DefineFont3, DefineFontAlignZones, DefineFontName, DefineEditText\n";
$tags = replace(swf(8, $head . font(75, 1)
                    . tag(73, pack('v', 1) . "\x00")              // DefineFontAlignZones
                    . tag(88, pack('v', 1) . "A\x00c\x00")         // DefineFontName
                    . tag(37, pack('v', 2) . "\x00" . "\x01\x00" . pack('vv', 1, 240) . "v\x00") // HasFont
                    . tag(26, "\x02" . pack('vv', 1, 2)) . $end));
foreach (added($tags) as $t) {
    if ($t[0] == 37) {
        $rect = intdiv(5 + 4 * (ord($t[1][2]) >> 3) + 7, 8);
        echo $t[0], ": id=", u16($t[1]), " font=", u16($t[1], 2 + $rect + 2), "\n";
    } else {
        echo $t[0], ": id=", u16($t[1]), "\n";
    }
}

echo "* DefineFont2, DefineText, DefineText2\n";
// DefineText(2): TEXTRECORDs (font, color, x, 1 glyph) (y) (font, 2 glyphs) ; GlyphBits = AdvanceBits = 1
function text($code, $id, $font) {
    $color = ($code == 33) ? "\xff\x00\x00\x80" : "\xff\x00\x00";
    return tag($code, pack('v', $id) . "\x00" . "\x00" . "\x01\x01"
               . "\x8d" . pack('v', $font) . $color . pack('vv', 0, 240) . "\x01" . "\x00"
               . "\x82" . pack('v', 20) . "\x00"
               . "\x88" . pack('vv', $font, 120) . "\x02" . "\x00"
               . "\x00");
}
$tags = replace(swf(8, $head . font(48, 1) . text(11, 2, 1) . text(33, 3, 1)
                    . tag(26, "\x02" . pack('vv', 1, 2)) . $end));
foreach (added($tags) as $t) {
    if (($t[0] == 11) || ($t[0] == 33)) {
        $n = ($t[0] == 33) ? 4 : 3;
        echo $t[0], ": id=", u16($t[1]), " fonts=", u16($t[1], 7), ",", u16($t[1], 20 + $n), "\n";
        var_dump(tag($t[0], $t[1]) === text($t[0], u16($t[1]), 8));
    } else {
        echo $t[0], ": id=", u16($t[1]), "\n";
    }
}

echo "* DefineButton2, DefineButton, DefineButtonCxform, DefineButtonSound\n";
// BUTTONRECORD (up, over) and (hit, with FilterList [Blur] and BlendMode)
$record2 = "\x03" . pack('vv', 3, 1) . "\x00" . "\x00"
         . "\x38" . pack('vv', 3, 2) . "\x00" . "\x00" . "\x01" . "\x01" . str_repeat("\x00", 9) . "\x02";
$tags = replace(swf(8, $head . shape(3)
                    . tag(34, pack('v', 4) . "\x00" . pack('v', 0) . $record2 . "\x00")
                    . tag(7, pack('v', 5) . "\x0f" . pack('vv', 3, 1) . "\x00" . "\x00" . "\x00") // + no action
                    . tag(23, pack('v', 5) . "\x00")                                           // DefineButtonCxform
                    . tag(14, pack('vCV', 6, 0x0a, 0))                                          // DefineSound
                    . tag(17, pack('vv', 5, 6) . "\x04" . pack('v', 2) . pack('vvv', 0, 0, 0))  // DefineButtonSound
                    . tag(26, "\x02" . pack('vv', 1, 4)) . $end));
foreach (added($tags) as $t) {
    switch ($t[0]) {
      case 34: echo "34: id=", u16($t[1]), " characters=", u16($t[1], 6), ",", u16($t[1], 6 + 7), "\n"; break;
      case 7:  echo "7: id=", u16($t[1]), " character=", u16($t[1], 3), "\n"; break;
      case 17: echo "17: button=", u16($t[1]), " sound=", u16($t[1], 2), " ", bin2hex(substr($t[1], 4)), "\n"; break;
      default: echo $t[0], ": id=", u16($t[1]), "\n";
    }
}

echo "* DoInitAction\n";
$tags = replace(swf(8, $head . tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, ''))
                    . tag(59, pack('v', 1) . "\x00")
                    . tag(26, "\x02" . pack('vv', 1, 1)) . $end));
foreach (added($tags) as $t) {
    echo $t[0], ": id=", u16($t[1]), "\n";
}
foreach ($tags as $t) {
    if (($t[0] == 39) && (u16($t[1]) == 7)) {
        echo "miku: ", implode(',', array_map(function ($t) { return $t[0]; }, tags_in($t[1], 4))), "\n";
    }
}

echo "* refers to a character the SWF doesn't define\n";
replace(swf(8, $head . tag(26, "\x02" . pack('vv', 1, 3)) . $end));               // PlaceObject2
replace(swf(8, $head . tag(37, pack('v', 2) . "\x00" . "\x01\x00" . pack('vv', 1, 240) . "v\x00") . $end));
replace(swf(8, $head . text(11, 2, 1) . $end));
replace(swf(8, $head . tag(34, pack('v', 4) . "\x00" . pack('v', 0) . $record2 . "\x00") . $end));
replace(swf(8, $head . tag(59, pack('v', 1) . "\x00") . $end));                     // DoInitAction
?>
===DONE===
--EXPECTF--
* DefineFont3, DefineFontAlignZones, DefineFontName, DefineEditText
bool(true)
75: id=8
73: id=8
88: id=8
37: id=9 font=8
* DefineFont2, DefineText, DefineText2
bool(true)
48: id=8
11: id=9 fonts=8,8
bool(true)
33: id=10 fonts=8,8
bool(true)
* DefineButton2, DefineButton, DefineButtonCxform, DefineButtonSound
bool(true)
2: id=8
34: id=9 characters=8,8
7: id=10 character=8
23: id=10
14: id=11
17: button=10 sound=11 040200000000000000
* DoInitAction
bool(true)
39: id=8
59: id=8
miku: 26,1,0
* refers to a character the SWF doesn't define
%Abool(false)
bool(true)
%Abool(false)
bool(true)
%Abool(false)
bool(true)
%Abool(false)
bool(true)
%Abool(false)
bool(true)
===DONE===
