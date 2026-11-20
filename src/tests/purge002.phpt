--TEST--
purgeUselessContents() keeps contents referenced from PlaceObject3 inside a sprite, and from DoInitAction after rebuild()
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
function lossless($code, $id) {
    return tag($code, pack('vCvv', $id, 5, 1, 1) . gzcompress("\xff\xff\x00\x00"));
}
// DefineShape(id) filled with bitmap
function shape($id, $bitmap) {
    $b = new Bits();
    $b->ub(4, 1); $b->ub(4, 0);                    // NumFillBits, NumLineBits
    $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 0); $b->ub(1, 1); $b->ub(1, 1);
    $b->ub(5, 2); $b->sb(2, 0); $b->sb(2, 0);      // MoveTo(0,0)
    $b->ub(1, 1);                                  // FillStyle0
    foreach (array(array(400, 0), array(0, 400), array(-400, 0), array(0, -400)) as $d) {
        $b->ub(1, 1); $b->ub(1, 1); $b->ub(4, 10); $b->ub(1, 1); $b->sb(12, $d[0]); $b->sb(12, $d[1]);
    }
    $b->ub(1, 0); $b->ub(5, 0);
    return tag(2, pack('v', $id) . "\x00" . "\x01\x41" . pack('v', $bitmap) . "\x00" . "\x00" . $b->bytes());
}
// DefineSprite(id) that places $child with PlaceObject2 / PlaceObject3
function sprite($id, $child, $place) {
    $inner = ($place == 3) ? tag(70, "\x02\x00" . pack('vv', 1, $child))   // PlaceObject3
                           : tag(26, "\x02" . pack('vv', 1, $child));      // PlaceObject2
    return tag(39, pack('vv', $id, 1) . $inner . tag(1, '') . tag(0, ''));
}
function names($o) {
    return implode(',', array_map(function ($t) {
        return isset($t['tagName']) ? $t['tagName'] : '(' . $t['code'] . ')';
    }, $o->getTagList()));
}

$attrs = tag(69, pack('V', 0));
$bg = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
$place = function ($id) { return tag(26, "\x02" . pack('vv', 1, $id)); };   // PlaceObject2
$doinit = function ($sprite) { return tag(59, pack('v', $sprite) . "\x00"); }; // DoInitAction(SpriteID, End)

$cases = array(
    // the bitmap(1) and the shape(2) are used only by PlaceObject3 in the sprite(3)
    'PlaceObject3 in sprite' => array(swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1) . sprite(3, 2, 3)
                                          . $place(3) . $end), false),
    // DefineBitsLossless(5) and DefineShape(6) are not referenced: still purged
    'unused ones are purged' => array(swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1) . lossless(20, 5) . shape(6, 5)
                                          . sprite(3, 2, 3) . $place(3) . $end), false),
    // the sprite(3) is used only by DoInitAction
    'DoInitAction'           => array(swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1) . sprite(3, 2, 2)
                                          . $doinit(3) . $end), false),
    // after rebuild() the raw data of DoInitAction is gone: its SpriteID is in the detail
    'DoInitAction, rebuilt'  => array(swf(8, $attrs . $bg . lossless(20, 1) . shape(2, 1) . sprite(3, 2, 2)
                                          . $doinit(3) . $end), true),
);
foreach ($cases as $name => $case) {
    $o = new SWFEditor();
    $o->input($case[0]);
    if ($case[1]) {
        var_dump($o->rebuild());
    }
    $o->purgeUselessContents();
    echo $name, ": ", names($o), "\n";
}
?>
===DONE===
--EXPECT--
PlaceObject3 in sprite: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,PlaceObject2,ShowFrame,End
unused ones are purged: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,PlaceObject2,ShowFrame,End
DoInitAction: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,DoInitAction,ShowFrame,End
bool(true)
DoInitAction, rebuilt: FileAttributes,SetBackgroundColor,DefineBitsLossless,DefineShape,DefineSprite,DoInitAction,ShowFrame,End
===DONE===
