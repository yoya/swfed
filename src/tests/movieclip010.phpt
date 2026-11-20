--TEST--
replaceMovieClip() keeps PlaceObject3, sound stream, scaling grid, binary data, exports and JPEGTables
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
function codes($tags) {
    return implode(',', array_map(function ($t) { return $t[0]; }, $tags));
}
function u16($s, $pos = 0) {
    return unpack('v', substr($s, $pos, 2))[1];
}
// the movie clip "miku" of negimiku.swf (DefineSprite id=7)
function miku($tags) {
    foreach ($tags as $t) {
        if (($t[0] == 39) && (u16($t[1]) == 7)) {
            return tags_in($t[1], 4);
        }
    }
    return null;
}
function replace($child) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf'));
    $o->setHeaderInfo(array('compress' => false));
    var_dump($o->replaceMovieClip('miku', $child));
    return tags($o->output());
}

$head = tag(69, pack('V', 0)) . tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
// DefineShape with a solid fill
function shape($id) {
    return tag(2, pack('v', $id) . "\x00" . "\x01\x00\xff\x00\x00" . "\x00" . "\x10" . "\x0a\x00");
}

echo "* PlaceObject3\n";
$tags = replace(swf(8, $head . shape(1) . tag(70, "\x02\x00" . pack('vv', 1, 1)) . $end));
$mc = miku($tags);
echo codes($mc), "\n";
echo "CharacterId=", u16($mc[0][1], 4), "\n"; // DefineShape: 1 => 8

echo "* sound stream, scaling grid, binary data, exports, metadata\n";
$block = tag(19, "\x00\x00\x00\x00");
$tags = replace(swf(8, $head . tag(77, '<rdf:RDF/>')
                   . tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, ''))  // => 8
                   . tag(78, pack('v', 1) . "\x00")                       // DefineScalingGrid
                   . tag(87, pack('vV', 2, 0) . 'hello')                  // DefineBinaryData => 9
                   . tag(56, pack('vv', 1, 1) . "linked\x00")             // ExportAssets
                   . tag(18, "\x0a\x1a" . pack('v', 100))                 // SoundStreamHead
                   . $block . tag(1, '') . $block . $end));
echo codes($tags), "\n";
foreach ($tags as $t) {
    switch ($t[0]) {
      case 78: echo "DefineScalingGrid CharacterId=", u16($t[1]), "\n"; break;
      case 87: echo "DefineBinaryData id=", u16($t[1]), " ", substr($t[1], 6), "\n"; break;
      case 56: echo "ExportAssets id=", u16($t[1], 2), " ", substr($t[1], 4, -1), "\n"; break;
    }
}
echo codes(miku($tags)), "\n";

echo "* JPEGTables\n";
$jpeg = file_get_contents('tests/5.jpg');
$tables = "\xff\xd8";
$image = "\xff\xd8";
for ($pos = 2 ; $pos < strlen($jpeg) ; ) {
    $marker = ord($jpeg[$pos + 1]);
    if ($marker == 0xda) { // SOS ... EOI
        $image .= substr($jpeg, $pos);
        break;
    }
    $seg = substr($jpeg, $pos, 2 + unpack('n', substr($jpeg, $pos + 2, 2))[1]);
    if (($marker == 0xdb) || ($marker == 0xc4)) { // DQT, DHT
        $tables .= $seg;
    } else {
        $image .= $seg;
    }
    $pos += strlen($seg);
}
$child = swf(8, $head . tag(8, $tables . "\xff\xd9") . tag(6, pack('v', 1) . $image)
                . tag(26, "\x02" . pack('vv', 1, 1)) . $end);
$c = new SWFEditor();
$c->input($child);
$orig = $c->getJpegData(1);
var_dump(strlen($orig) > 0);
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->replaceMovieClip('miku', $child));
$o2 = new SWFEditor();
$o2->input($o->output());
foreach ($o2->getTagList() as $n => $t) {
    $d = $o2->getTagDetail($n);
    if (isset($d['image_id']) && ($d['image_id'] == 8)) {
        echo $t['tagName'], "\n";
    }
}
var_dump($o2->getJpegData(8) === $orig);

echo "* unsupported tag\n";
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
$before = $o->output();
var_dump($o->replaceMovieClip('miku', swf(9, $head . tag(82, pack('V', 1) . "\x00" . "abc") . $end)));
var_dump($o->output() === $before);

echo "* search PlaceObject3 by instance name\n";
// HasName, HasMatrix, HasCharacter / HasClassName, HasBlendMode
$place3 = tag(70, "\x26\x0a" . pack('v', 1) . "Foo\x00" . pack('v', 1) . "\x00" . "mc\x00" . "\x01");
$o = new SWFEditor();
$o->input(swf(8, $head . tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, '')) . $place3 . $end));
var_dump($o->replaceMovieClip('mc', file_get_contents('tests/saitama.swf')));
$tags = tags($o->output());
foreach ($tags as $t) {
    if ($t[0] == 39) {
        echo "DefineSprite id=", u16($t[1]), " frames=", u16($t[1], 2), "\n";
    } elseif ($t[0] == 70) {
        var_dump($t[1] === substr($place3, 2));
    }
}
?>
===DONE===
--EXPECTF--
* PlaceObject3
bool(true)
70,1,0
CharacterId=8
* sound stream, scaling grid, binary data, exports, metadata
bool(true)
9,8,6,2,26,35,2,35,2,39,78,87,56,39,26,1,0
DefineScalingGrid CharacterId=8
DefineBinaryData id=9 hello
ExportAssets id=8 linked
18,19,1,19,1,0
* JPEGTables
bool(true)
bool(true)
DefineBitsJPEG2
bool(true)
* unsupported tag
%Abool(false)
bool(true)
* search PlaceObject3 by instance name
bool(true)
DefineSprite id=1 frames=%d
bool(true)
===DONE===
