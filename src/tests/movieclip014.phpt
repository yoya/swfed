--TEST--
removeTag() in a sprite that replaceMovieClip() filled removes that tag only (#148)
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
// DefineShape with a solid fill and no edges
function shape($id) {
    return tag(2, pack('v', $id) . "\x00" . "\x01" . "\x00\xff\x00\x00" . "\x00" . "\x10" . "\x00");
}
function place($depth, $id) { // PlaceObject2: HasCharacter
    return tag(26, "\x02" . pack('vv', $depth, $id));
}
// the tags in the DefineSprite at $index, as "code:depth" (PlaceObject2) or "code"
function inner($o, $index) {
    $d = $o->getTagData($index);
    $p = (((ord($d[0]) | (ord($d[1]) << 8)) & 0x3f) == 0x3f) ? 6 + 4 : 2 + 4;
    $r = array();
    while ($p + 2 <= strlen($d)) {
        $v = ord($d[$p]) | (ord($d[$p + 1]) << 8);
        $p += 2;
        $len = $v & 0x3f;
        if ($len == 0x3f) {
            $len = unpack('V', substr($d, $p, 4))[1];
            $p += 4;
        }
        $r[] = (($v >> 6) == 26) ? ('26:' . (ord($d[$p + 1]) | (ord($d[$p + 2]) << 8))) : ($v >> 6);
        $p += $len;
    }
    return implode(' ', $r);
}
function filled() {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf'));
    $child = swf(4, shape(1) . place(1, 1) . tag(1, '') . place(2, 1) . tag(1, '') . place(3, 1) . tag(1, '') . tag(0, ''));
    var_dump($o->replaceMovieClip('miku', $child));
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 39) {
            $sprite = $i;
        }
    }
    return array($o, $sprite);
}

list($o, $sprite) = filled();
echo "filled: ", inner($o, $sprite), "\n";
foreach (array(3 => 'a tag in the middle', 0 => 'the first tag', 5 => 'the last tag') as $i => $what) {
    list($o, $sprite) = filled();
    var_dump($o->removeTag($sprite, $i));
    echo "$what ($i): ", inner($o, $sprite), "\n";
}
// one after another, and the result is read back
list($o, $sprite) = filled();
var_dump($o->removeTag($sprite, 2), $o->removeTag($sprite, 2), $o->removeTag($sprite, 100));
echo "two tags: ", inner($o, $sprite), "\n";
$o2 = new SWFEditor();
var_dump($o2->input($o->output()));
echo "read back: ", inner($o2, $sprite), "\n";
?>
===DONE===
--EXPECT--
bool(true)
filled: 26:1 1 26:2 1 26:3 1 0
bool(true)
bool(true)
a tag in the middle (3): 26:1 1 26:2 26:3 1 0
bool(true)
bool(true)
the first tag (0): 1 26:2 1 26:3 1 0
bool(true)
bool(true)
the last tag (5): 26:1 1 26:2 1 26:3 0
bool(true)
bool(true)
bool(true)
bool(false)
two tags: 26:1 1 26:3 1 0
bool(true)
read back: 26:1 1 26:3 1 0
===DONE===
