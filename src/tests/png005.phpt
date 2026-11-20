--TEST--
getPNGData() un-premultiplies Lossless2 colors and keeps alpha 254 in tRNS
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
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
function chunk($type, $data) {
    return pack('N', strlen($data)) . $type . $data . pack('N', crc32($type . $data));
}
// minimal decoder for 8bit RGBA / palette PNG
function png_decode($png) {
    $pos = 8; $c = array('IDAT' => '');
    while ($pos < strlen($png)) {
        $len = unpack('N', substr($png, $pos, 4))[1];
        $type = substr($png, $pos + 4, 4);
        $data = substr($png, $pos + 8, $len);
        $c[$type] = ($type === 'IDAT') ? $c['IDAT'] . $data : $data;
        $pos += 12 + $len;
    }
    $h = unpack('Nw/Nh/Cdepth/Ctype', $c['IHDR']);
    $bpp = ($h['type'] == 6) ? 4 : 1;
    $raw = gzuncompress($c['IDAT']);
    $stride = $h['w'] * $bpp;
    $prev = array_fill(0, $stride, 0);
    $px = array();
    for ($y = 0; $y < $h['h']; $y++) {
        $f = ord($raw[$y * ($stride + 1)]);
        $row = array();
        for ($i = 0; $i < $stride; $i++) {
            $x = ord($raw[$y * ($stride + 1) + 1 + $i]);
            $a = ($i >= $bpp) ? $row[$i - $bpp] : 0;
            $b = $prev[$i];
            $cc = ($i >= $bpp) ? $prev[$i - $bpp] : 0;
            switch ($f) {
            case 1: $x += $a; break;
            case 2: $x += $b; break;
            case 3: $x += ($a + $b) >> 1; break;
            case 4:
                $p = $a + $b - $cc;
                $pa = abs($p - $a); $pb = abs($p - $b); $pc = abs($p - $cc);
                $x += ($pa <= $pb && $pa <= $pc) ? $a : (($pb <= $pc) ? $b : $cc);
                break;
            }
            $row[$i] = $x & 0xff;
        }
        $px = array_merge($px, $row);
        $prev = $row;
    }
    return array('type' => $h['type'], 'PLTE' => isset($c['PLTE']) ? bin2hex($c['PLTE']) : '',
                 'tRNS' => isset($c['tRNS']) ? bin2hex($c['tRNS']) : '', 'pixels' => $px);
}

// Lossless2 format=5, straight colors via replacePNGData (RGBA PNG)
$rgba = "\xff\x00\x00\x80" . "\x64\xc8\x32\x4d" . "\x00\x00\xff\xff" . "\x00\x00\x00\x00";
$png = "\x89PNG\r\n\x1a\n"
     . chunk('IHDR', pack('NNCCCCC', 4, 1, 8, 6, 0, 0, 0))
     . chunk('IDAT', gzcompress("\x00" . $rgba))
     . chunk('IEND', '');
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
var_dump($o->replacePNGData(1, $png));
$d = png_decode($o->getPNGData(1));
echo $d['type'], " ", implode(',', $d['pixels']), "\n";

// Lossless2 format=3, premultiplied RGBA colormap; last color has alpha 254
$colormap = "\x80\x00\x00\x80" . "\x00\x00\xff\xff" . "\x00\xfe\x00\xfe";
$bits = pack('vCvvC', 1, 3, 3, 1, 2) . gzcompress($colormap . "\x00\x01\x02\x00");
$o->input(swf(8, tag(9, "\xff\xff\xff") . tag(36, $bits) . tag(1, '') . tag(0, '')));
$d = png_decode($o->getPNGData(1));
echo $d['type'], " PLTE=", $d['PLTE'], " tRNS=", $d['tRNS'], " ", implode(',', $d['pixels']), "\n";
?>
===DONE===
--EXPECT--
bool(true)
6 255,0,0,128,99,199,50,77,0,0,255,255,0,0,0,0
3 PLTE=ff00000000ff00ff00 tRNS=80fffe 0,1,2
===DONE===
