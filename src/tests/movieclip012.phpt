--TEST--
replaceMovieClip() follows the CID change in RemoveObject (not in RemoveObject2)
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
// the tags of a movie clip: PlaceObject2 (depth, char), RemoveObject (char, depth), RemoveObject2 (depth)
function show($tags, $indent = '') {
    foreach ($tags as $t) {
        switch ($t[0]) {
          case 26: echo $indent, "26: depth=", u16($t[1], 1), " char=", u16($t[1], 3), "\n"; break;
          case 5:  echo $indent, "5: char=", u16($t[1]), " depth=", u16($t[1], 2), "\n"; break;
          case 28: echo $indent, "28: depth=", u16($t[1]), "\n"; break;
          case 2:  echo $indent, "2: id=", u16($t[1]), "\n"; break;
          case 39:
            echo $indent, "39: id=", u16($t[1]), "\n";
            show(tags_in($t[1], 4), $indent . '  ');
            break;
        }
    }
}
// the tags of the movie clip "miku" (DefineSprite id=7) of negimiku.swf, and the tags added in front of it
function replace($child) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf')); // CID 1-7
    $before = $o->output();
    $ret = $o->replaceMovieClip('miku', $child);
    var_dump($ret);
    $out = $o->output();
    if (! $ret) {
        var_dump($out === $before);
        return;
    }
    $added = array();
    foreach (array_slice(tags($out), 9) as $t) {
        if (($t[0] == 39) && (u16($t[1]) == 7)) {
            echo "miku\n";
            show(tags_in($t[1], 4));
            break;
        }
        $added[] = $t;
    }
    echo "added\n";
    show($added);
}

$head = tag(69, pack('V', 0)) . tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
// DefineShape with a solid fill
function shape($id) {
    return tag(2, pack('v', $id) . "\x00" . "\x01\x00\xff\x00\x00" . "\x00" . "\x10" . "\x0a\x00");
}
function place($depth, $cid) {
    return tag(26, "\x02" . pack('vv', $depth, $cid));
}

echo "* RemoveObject\n";
// the depth of RemoveObject2 is not a character ID, though it equals one (1)
replace(swf(8, $head . shape(1) . place(1, 1) . place(2, 1) . tag(1, '')
            . tag(5, pack('vv', 1, 2))   // RemoveObject: CharacterId=1, Depth=2
            . tag(28, pack('v', 1))      // RemoveObject2: Depth=1
            . $end));

echo "* RemoveObject in a DefineSprite\n";
replace(swf(8, $head . shape(1)
            . tag(39, pack('vv', 2, 2) . place(1, 1) . tag(1, '') . tag(5, pack('vv', 1, 1)) . tag(1, '') . tag(0, ''))
            . place(1, 2) . $end));

echo "* RemoveObject of a character the SWF doesn't define\n";
replace(swf(8, $head . shape(1) . place(1, 1) . tag(5, pack('vv', 3, 1)) . $end));
?>
===DONE===
--EXPECTF--
* RemoveObject
bool(true)
miku
26: depth=1 char=8
26: depth=2 char=8
5: char=8 depth=2
28: depth=1
added
2: id=8
* RemoveObject in a DefineSprite
bool(true)
miku
26: depth=1 char=9
added
2: id=8
39: id=9
  26: depth=1 char=8
  5: char=8 depth=1
* RemoveObject of a character the SWF doesn't define
%Abool(false)
bool(true)
===DONE===
