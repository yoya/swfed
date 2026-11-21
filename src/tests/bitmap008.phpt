--TEST--
DefineBitsLossless / 2 (format 5) whose data is shorter than 4 * width * height is refused (#144)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function tag($code, $body) {
    return pack('vV', ($code << 6) | 0x3f, strlen($body)) . $body;
}
function swf($tags) {
    // FrameSize 550x400 pixels, FrameRate 12, FrameCount 1
    $body = "\x78\x00\x05\x5f\x00\x00\x0f\xa0\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr(6) . pack('V', 8 + strlen($body)) . $body;
}
function lossless($code, $id, $w, $h, $pixels) {
    // bytes after the zlib stream make the tag long enough for the size it claims
    return tag($code, pack('v', $id) . chr(5) . pack('vv', $w, $h) . gzcompress($pixels) . str_repeat("\0", 64));
}

foreach (array(20 => 'DefineBitsLossless', 36 => 'DefineBitsLossless2') as $code => $name) {
    echo "== $name\n";
    // 8x8 = 64 pixels: one pixel, all but one pixel, all the pixels
    foreach (array(4, 4 * 63, 4 * 64) as $len) {
        $o = new SWFEditor();
        $o->input(swf(lossless($code, 1, 8, 8, str_repeat("\x00\x10\x20\x30", $len / 4)) . tag(1, '') . tag(0, '')));
        $detail = $o->getTagDetail(0);
        echo "$len bytes: detail=", is_array($detail) ? "{$detail['width']}x{$detail['height']}" : var_export($detail, true),
             " rebuild=", var_export($o->rebuild(), true);
        if (is_array($detail)) {
            $o2 = new SWFEditor();
            $o2->input($o->output());
            echo " pixels kept=", var_export(gzuncompress(substr($o2->getTagData(0), 6 + 7)) === str_repeat("\x00\x10\x20\x30", 64), true);
        } else {
            echo " error=", (strpos((string) SWFEditor::getLastError(), 'data size') !== false) ? 'data size' : 'other';
        }
        echo "\n";
    }
}
?>
===DONE===
--EXPECT--
== DefineBitsLossless
4 bytes: detail=false rebuild=false error=data size
252 bytes: detail=false rebuild=false error=data size
256 bytes: detail=8x8 rebuild=true pixels kept=true
== DefineBitsLossless2
4 bytes: detail=false rebuild=false error=data size
252 bytes: detail=false rebuild=false error=data size
256 bytes: detail=8x8 rebuild=true pixels kept=true
===DONE===
