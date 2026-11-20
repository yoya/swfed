--TEST--
replaceGIFData() / replacePNGData() / Lossless input reject palette index overflow
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); $info = ob_get_clean();
if (strpos($info, 'PNG support => yes') === false) die('skip swfed built without libpng');
if (strpos($info, 'GIF support => yes') === false) die('skip swfed built without giflib'); ?>
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
// 2 color GIF; LZW codes are 3 bits and a clear code precedes every pixel
function gif($pixels) {
    $codes = array();
    foreach ($pixels as $p) {
        $codes[] = 4; // clear
        $codes[] = $p;
    }
    $codes[] = 5; // end
    $bits = 0; $n = 0; $lzw = '';
    foreach ($codes as $c) {
        $bits |= $c << $n;
        $n += 3;
        while ($n >= 8) { $lzw .= chr($bits & 0xff); $bits >>= 8; $n -= 8; }
    }
    if ($n > 0) { $lzw .= chr($bits & 0xff); }
    return "GIF89a" . pack('vv', count($pixels), 1) . "\x80\x00\x00"
         . "\xff\x00\x00" . "\x00\x00\xff"
         . "\x2c" . pack('vvvv', 0, 0, count($pixels), 1) . "\x00"
         . "\x02" . chr(strlen($lzw)) . $lzw . "\x00" . "\x3b";
}
function chunk($type, $data) {
    return pack('N', strlen($data)) . $type . $data . pack('N', crc32($type . $data));
}
// 2 color palette PNG (8 bit depth)
function png($pixels) {
    return "\x89PNG\r\n\x1a\n"
         . chunk('IHDR', pack('NNCCCCC', count($pixels), 1, 8, 3, 0, 0, 0))
         . chunk('PLTE', "\xff\x00\x00" . "\x00\x00\xff")
         . chunk('IDAT', gzcompress("\x00" . implode('', array_map('chr', $pixels))))
         . chunk('IEND', '');
}

$saitama = file_get_contents('tests/saitama.swf');
$obj = new SWFEditor();

$obj->input($saitama);
var_dump($obj->replaceGIFData(1, gif(array(0, 1, 1, 0))));
var_dump(strlen($obj->getPNGData(1)) > 0);
$obj->input($saitama);
var_dump($obj->replaceGIFData(1, gif(array(3, 1, 1, 0))));

$obj->input($saitama);
var_dump($obj->replacePNGData(1, png(array(0, 1, 1, 0))));
var_dump(strlen($obj->getPNGData(1)) > 0);
$obj->input($saitama);
var_dump($obj->replacePNGData(1, png(array(0, 1, 5, 0))));

// DefineBitsLossless format=3 in the SWF itself (2 colors, index 5)
$bits = pack('vCvvC', 1, 3, 4, 1, 1)
      . gzcompress("\xff\x00\x00" . "\x00\x00\xff" . "\x00\x01\x05\x00");
$obj->input(swf(6, tag(9, "\xff\xff\xff") . tag(20, $bits) . tag(1, '') . tag(0, '')));
var_dump($obj->getPNGData(1));
var_dump(@$obj->replaceBitmapData(array('red' => 255, 'green' => 0, 'blue' => 0),
                                  file_get_contents('tests/negimiku001.png')));
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
%Abool(false)
bool(true)
bool(true)
%Abool(false)
%Abool(false)
%Abool(false)
===DONE===
