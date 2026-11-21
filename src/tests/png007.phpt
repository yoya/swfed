--TEST--
getBitmapSize() of a PNG in a DefineBitsJPEG2: a width / height that is 0 or 2^31 and over is refused (#156)
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
// only the signature and the IHDR chunk are looked at
function png($w, $h) {
    return "\x89PNG\r\n\x1a\n" . pack('N', 13) . 'IHDR' . pack('NNCCCCC', $w, $h, 8, 2, 0, 0, 0) . pack('N', 0);
}

foreach (array(array(1, 1), array(640, 480), array(0x7fffffff, 0x7fffffff), array(0x80000000, 1), array(1, 0x80000000),
               array(0xffffffff, 0xffffffff), array(0xff000000, 2), array(0, 1), array(1, 0)) as $size) {
    $o = new SWFEditor();
    $o->input(swf(8, tag(21, pack('v', 1) . png($size[0], $size[1])) . tag(1, '') . tag(0, '')));
    $r = $o->getBitmapSize(1);
    printf("%ux%u: %s\n", $size[0], $size[1], is_array($r) ? "{$r['width']}x{$r['height']}" : var_export($r, true));
}
?>
===DONE===
--EXPECT--
1x1: 1x1
640x480: 640x480
2147483647x2147483647: 2147483647x2147483647
2147483648x1: false
1x2147483648: false
4294967295x4294967295: false
4278190080x2: false
0x1: false
1x0: false
===DONE===
