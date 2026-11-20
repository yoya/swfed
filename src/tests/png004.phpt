--TEST--
replacePNGData() accepts gray / gray+alpha / 16bit PNG and keeps tRNS transparency
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--FILE--
<?php

function chunk($type, $data) {
    return pack('N', strlen($data)) . $type . $data . pack('N', crc32($type . $data));
}
// one row PNG; $row is the raw scanline without the filter byte
function png($width, $depth, $color_type, $row, $extra = '') {
    return "\x89PNG\r\n\x1a\n"
         . chunk('IHDR', pack('NNCCCCC', $width, 1, $depth, $color_type, 0, 0, 0))
         . $extra
         . chunk('IDAT', gzcompress("\x00" . $row))
         . chunk('IEND', '');
}
// tag code and pixels (RRGGBBAA, premultiplied as stored) of the Lossless bitmap
function lossless_pixels($o) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == 20 || $t['code'] == 36) {
            $data = $o->getTagData($i);
            $body = substr($data, ((ord($data[0]) & 0x3f) == 0x3f) ? 6 : 2);
            $h = unpack('vid/Cformat/vwidth/vheight', $body);
            $raw = gzuncompress(substr($body, 7));
            $px = array();
            for ($x = 0; $x < $h['width']; $x++) {
                $p = substr($raw, $x * 4, 4);
                $a = ($t['code'] == 36) ? bin2hex($p[0]) : '--';
                $px[] = bin2hex(substr($p, 1)) . $a;
            }
            return $t['code'] . ' ' . implode(' ', $px);
        }
    }
    return 'none';
}

$images = array(
    'gray 8bit'       => png(4, 8, 0, "\x00\x80\xff\x40"),
    'gray 1bit'       => png(4, 1, 0, "\xb0"),
    'gray+alpha 8bit' => png(3, 8, 4, "\xff\xff\x00\x00\x80\x80"),
    'rgb 16bit'       => png(1, 16, 2, "\xff\xff\x80\x00\x00\x00"),
    'rgba 16bit'      => png(2, 16, 6, "\xff\xff\x00\x00\x00\x00\xff\xff" . "\x00\x00\xff\xff\x00\x00\x80\x00"),
    'rgb + tRNS'      => png(2, 8, 2, "\xff\x00\x00" . "\x00\x00\xff", chunk('tRNS', pack('nnn', 0, 0, 255))),
    'gray + tRNS'     => png(2, 8, 0, "\x00\xc8", chunk('tRNS', pack('n', 0))),
);
$saitama = file_get_contents('tests/saitama.swf');
$o = new SWFEditor();
foreach ($images as $name => $png) {
    $o->input($saitama);
    $r = $o->replacePNGData(1, $png);
    echo $name, ": ", var_export($r, true), " ", $r ? lossless_pixels($o) : '', "\n";
}
?>
===DONE===
--EXPECT--
gray 8bit: true 20 000000-- 808080-- ffffff-- 404040--
gray 1bit: true 20 ffffff-- 000000-- ffffff-- ffffff--
gray+alpha 8bit: true 36 ffffffff 00000000 40404080
rgb 16bit: true 20 ff8000--
rgba 16bit: true 36 ff0000ff 00800080
rgb + tRNS: true 36 ff0000ff 00000000
gray + tRNS: true 36 00000000 c8c8c8ff
===DONE===
