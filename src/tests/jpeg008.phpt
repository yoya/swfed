--TEST--
JPEG scan data that is a single 0xFF at the end of the data does not get the length -1 (#157)
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

$sof = "\xff\xc0\x00\x11\x08\x00\x10\x00\x10\x03\x01\x11\x00\x02\x11\x00\x03\x11\x00"; // 16x16
$cases = array(
    'SOS, then 0xFF'             => "\xff\xd8\xff\xda\xff",
    'RST0, then 0xFF'            => "\xff\xd8\xff\xd0\xff",
    'SOF, SOS, then 0xFF'        => "\xff\xd8" . $sof . "\xff\xda\xff",
    'SOS, data, then 0xFF'       => "\xff\xd8" . $sof . "\xff\xda\x12\x34\xff",
    'SOS, data, no 0xFF'         => "\xff\xd8" . $sof . "\xff\xda\x12\x34",
    'SOS, data, EOI'             => "\xff\xd8" . $sof . "\xff\xda\x12\x34\xff\xd9",
);
foreach ($cases as $name => $jpeg) {
    $o = new SWFEditor();
    $o->input(swf(8, tag(21, pack('v', 1) . $jpeg) . tag(6, pack('v', 2) . $jpeg)
                     . tag(35, pack('v', 3) . pack('V', strlen($jpeg)) . $jpeg) . tag(1, '') . tag(0, '')));
    ob_start();
    $o->swfInfo();
    $info = ob_get_clean();
    preg_match_all('/len=(\d+)/', $info, $m);
    echo "$name: segment lengths ", implode(',', array_unique($m[1])),
         " size=", json_encode($o->getBitmapSize(1)), "\n";
    foreach (array(1, 2, 3) as $id) { // none of these may crash
        $o->getJpegData($id);
        $o->replaceJpegData($id, $jpeg);
        $o->replaceBitmapData($id, $jpeg);
    }
    $o->rebuild();
    $o->output();
}
?>
===DONE===
--EXPECT--
SOS, then 0xFF: segment lengths 0 size=false
RST0, then 0xFF: segment lengths 0 size=false
SOF, SOS, then 0xFF: segment lengths 0,15 size={"width":16,"height":16}
SOS, data, then 0xFF: segment lengths 0,15 size={"width":16,"height":16}
SOS, data, no 0xFF: segment lengths 0,15 size={"width":16,"height":16}
SOS, data, EOI: segment lengths 0,15,2 size={"width":16,"height":16}
===DONE===
