--TEST--
replaceMovieClip() rejects a shape whose bitmap fill refers to a bitmap the SWF doesn't define
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
function lossless($id, $width) {
    return tag(20, pack('vCvv', $id, 5, $width, 1) . gzcompress(str_repeat("\x00\xff\x00\x00", $width)));
}
// DefineShape(2) with a clipped bitmap fill and no edges
function shape($id, $bitmap) {
    return tag(2, pack('v', $id) . "\x00" . "\x01" . "\x41" . pack('v', $bitmap) . "\x00"
                  . "\x00" . "\x10" . "\x00");
}

function replace($child) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf'));   // uses CID 1..7
    $before = $o->output();
    var_dump($o->replaceMovieClip('miku', $child));
    $after = $o->output();
    if ($after === $before) {
        echo "not changed\n";
        return;
    }
    $bitmap = null;
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 20) {
            $d = $o->getTagDetail($i);
            if ($d['width'] == 3) {
                $bitmap = $d['image_id'];
            }
        }
    }
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 2) {
            $d = $o->getTagDetail($i);
            if (isset($d['bitmap_ref']) && ($d['bitmap_ref'] === array($bitmap))) {
                echo "the fill refers to the bitmap that was moved\n";
                return;
            }
        }
    }
    echo "wrong\n";
}

$head = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
$place = tag(26, "\x02" . pack('vv', 1, 2));

echo "* the bitmap is defined (CID 1 is used by negimiku.swf)\n";
replace(swf(6, $head . lossless(1, 3) . shape(2, 1) . $place . $end));

echo "* the bitmap is not defined\n";
replace(swf(6, $head . lossless(1, 3) . shape(2, 5) . $place . $end));
replace(swf(6, $head . shape(2, 1) . $place . $end));
?>
===DONE===
--EXPECTF--
* the bitmap is defined (CID 1 is used by negimiku.swf)
bool(true)
the fill refers to the bitmap that was moved
* the bitmap is not defined
%Abool(false)
not changed
%Abool(false)
not changed
===DONE===
