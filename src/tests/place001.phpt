--TEST--
PlaceObject2 keeps ClipActions (onClipEvent) after rebuild() / replaceMovieClip()
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
function place_data($o) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 26) {
            return $o->getTagData($i);
        }
    }
}

// CLIPACTIONS (SWF 6+): Reserved, AllEventFlags, one record (EventFlags, size, Stop, End), end flag
$clip = pack('vV', 0, 1) . pack('VV', 1, 2) . "\x07\x00" . pack('V', 0);
// HasClipActions|HasCharacter, depth=1, character=1
$place = tag(26, "\x82" . pack('vv', 1, 1) . $clip);
$sprite = tag(39, pack('vv', 1, 1) . tag(1, '') . tag(0, ''));
$swf = swf(6, tag(9, "\xff\xff\xff") . $sprite . $place . tag(1, '') . tag(0, ''));

$o = new SWFEditor();
$o->input($swf);
var_dump($o->rebuild());
var_dump(place_data($o) === $place);
$o2 = new SWFEditor();
$o2->input($o->output());
var_dump(place_data($o2) === $place);

// replaceMovieClip() renumbers the character id, which rebuilds the PlaceObject2
$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->replaceMovieClip('miku', $swf));
$out = $o->output();
var_dump(strpos($out, $clip) !== false);
var_dump($o2->input($out));
?>
===DONE===
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
