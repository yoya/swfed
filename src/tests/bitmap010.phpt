--TEST--
swfed.max_uncompressed_size also limits the PNG / GIF given to replacePNGData() / replaceGIFData() (#146)
--SKIPIF--
<?php
ob_start(); phpinfo(INFO_MODULES); $info = ob_get_clean();
if (strpos($info, 'PNG support => yes') === false) die('skip swfed built without libpng');
if (strpos($info, 'GIF support => yes') === false) die('skip swfed built without giflib');
?>
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function chunk($type, $d) {
    return pack('N', strlen($d)) . $type . $d . pack('N', crc32($type . $d));
}
function png($w, $h) { // RGB, all black
    return "\x89PNG\r\n\x1a\n" . chunk('IHDR', pack('NNCCCCC', $w, $h, 8, 2, 0, 0, 0))
         . chunk('IDAT', gzcompress(str_repeat("\0" . str_repeat("\0", 3 * $w), $h))) . chunk('IEND', '');
}
function gif($screen_w, $screen_h, $frames = 1) { // 1x1 frames on a logical screen
    $frame = "\x21\xf9\x04\x00\x00\x00\x00\x00"                        // graphic control extension
           . "\x2c" . pack('vvvv', 0, 0, 1, 1) . "\x00" . "\x02\x02\x44\x01\x00";
    return "GIF89a" . pack('vv', $screen_w, $screen_h) . "\x80\x00\x00" . "\x00\x00\x00\xff\xff\xff"
         . str_repeat($frame, $frames) . "\x3b";
}
function limit_error() {
    return (strpos((string) SWFEditor::getLastError(), 'exceeds the limit') !== false) ? 'limit' : 'no limit error';
}
function replace($method, $image) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/colorformat.swf'));
    $before = $o->output();
    $r = $o->$method(2, $image);
    $error = limit_error(); // of the replace: the next method call clears it
    $size = $o->getBitmapSize(2);
    return var_export($r, true) . ($r ? " {$size['width']}x{$size['height']}"
                                      : (" $error kept=" . var_export($o->output() === $before, true)));
}

$png = png(1000, 1000);              // 3 MB of rows
$gif = gif(2000, 2000);              // 4 MB of logical screen, in a few bytes
$gif_max = gif(0xffff, 0xffff);      // 4 GB
echo "sizes: png<64K=", var_export(strlen($png) < 65536, true), " gif=", strlen($gif), "\n";
foreach (array('1M', '16M', '-1') as $limit) {
    ini_set('swfed.max_uncompressed_size', $limit);
    echo "== $limit\n";
    echo "png 1000x1000: ", replace('replacePNGData', $png), "\n";
    echo "gif 2000x2000: ", replace('replaceGIFData', $gif), "\n";
    echo "gif by replaceBitmapData: ", replace('replaceBitmapData', $gif), "\n";
    if ($limit !== '-1') {
        echo "gif 65535x65535: ", replace('replaceGIFData', $gif_max), "\n";
    }
}
// the frames count too: 300 frames of 1x1 on a 16x16 screen
ini_set('swfed.max_uncompressed_size', '500');
echo "300 frames, limit 500: ", replace('replaceGIFData', gif(16, 16, 300)), "\n";
ini_set('swfed.max_uncompressed_size', '600');
echo "300 frames, limit 600: ", replace('replaceGIFData', gif(16, 16, 300)), "\n";
?>
===DONE===
--EXPECT--
sizes: png<64K=true gif=43
== 1M
png 1000x1000: false limit kept=true
gif 2000x2000: false limit kept=true
gif by replaceBitmapData: false limit kept=true
gif 65535x65535: false limit kept=true
== 16M
png 1000x1000: true 1000x1000
gif 2000x2000: true 2000x2000
gif by replaceBitmapData: true 2000x2000
gif 65535x65535: false limit kept=true
== -1
png 1000x1000: true 1000x1000
gif 2000x2000: true 2000x2000
gif by replaceBitmapData: true 2000x2000
300 frames, limit 500: false limit kept=true
300 frames, limit 600: true 16x16
===DONE===
