--TEST--
replaceMovieClip() refuses a SWF with a shape that can't be parsed: its bitmap fills could not be renumbered (#179)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

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
function lossless($id) {
    return tag(20, pack('vCvv', $id, 5, 1, 1) . gzcompress("\x00\xff\x00\x00"));
}
function place($depth, $id) {
    return tag(26, "\x02" . pack('vv', $depth, $id));
}
// DefineShape2 with a clipped bitmap fill of bitmap 1; $broken adds a fill style of an unknown type
function shape($broken) {
    return tag(22, pack('v', 2) . "\x00" . ($broken ? "\x02" : "\x01") . "\x41" . pack('v', 1) . "\x00"
                   . ($broken ? "\x77" : '') . "\x00" . "\x10" . "\x00");
}

foreach (array('shape' => false, 'shape that can\'t be parsed' => true) as $name => $broken) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/negimiku.swf'));   // uses CID 1..7
    $before = $o->output();
    $r = $o->replaceMovieClip('miku', swf(6, lossless(1) . shape($broken) . place(1, 2) . tag(1, '') . tag(0, '')));
    $error = (strpos((string) SWFEditor::getLastError(), 'broken shape') !== false) ? 'broken shape' : 'other';
    echo "$name: ", var_export($r, true);
    if ($r) {
        // the inserted bitmap got a new ID, and the fill of the inserted shape follows it
        $bitmap = $fill = null;
        foreach ($o->getTagList() as $i => $t) {
            $d = $o->getTagDetail($i);
            if (($t['code'] == 20) && ($d['width'] == 1)) {
                $bitmap = $d['image_id'];
            }
            if (($t['code'] == 22) && isset($d['bitmap_ref']) && ($d['fill_styles.count'] == 1) && ($d['shape_id'] > 7)) {
                $fill = $d['bitmap_ref'][0];
            }
        }
        echo " new bitmap ID > 7: ", var_export($bitmap > 7, true), ", the shape refers to it: ", var_export($fill === $bitmap, true), "\n";
    } else {
        echo " movie kept: ", var_export($o->output() === $before, true),
             " error: $error\n";
    }
}
?>
===DONE===
--EXPECT--
shape: true new bitmap ID > 7: true, the shape refers to it: true
shape that can't be parsed: false movie kept: true error: broken shape
===DONE===
