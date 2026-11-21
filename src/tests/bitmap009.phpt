--TEST--
swfed.max_uncompressed_size also limits the data of DefineBitsLossless and the alpha of DefineBitsJPEG3 (#146)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function tag($code, $body) {
    return pack('vV', ($code << 6) | 0x3f, strlen($body)) . $body;
}
function swf($tags) {
    $body = "\x78\x00\x05\x5f\x00\x00\x0f\xa0\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr(6) . pack('V', 8 + strlen($body)) . $body;
}
function limit_error() {
    return (strpos((string) SWFEditor::getLastError(), 'exceeds the limit') !== false) ? 'limit' : 'no limit error';
}

// 1000x1000: 4 MB as format 5, 1 MB (+ colormap) as format 3. The SWF is not compressed (FWS).
$w = $h = 1000;
$jpeg = file_get_contents('tests/5.jpg');
$data = swf(tag(20, pack('v', 1) . chr(5) . pack('vv', $w, $h) . gzcompress(str_repeat("\0", 4 * $w * $h)))
          . tag(36, pack('v', 2) . chr(3) . pack('vv', $w, $h) . chr(0) . gzcompress(str_repeat("\0", 4 + $w * $h)))
          . tag(35, pack('v', 3) . pack('V', strlen($jpeg)) . $jpeg . gzcompress(str_repeat("\xff", 4 * 1048576)))
          . "\x40\x00" . "\x00\x00"); // ShowFrame, End
echo "SWF is smaller than 512K: ", var_export(strlen($data) < 512 * 1024, true), "\n";

foreach (array('512K', '2M', '8M', '-1') as $limit) {
    ini_set('swfed.max_uncompressed_size', $limit);
    $o = new SWFEditor();
    echo "== $limit: input=", var_export($o->input($data), true), "\n";
    $d = $o->getTagDetail(0);
    echo "format 5: ", is_array($d) ? "{$d['width']}x{$d['height']}" : (var_export($d, true) . ' ' . limit_error()), "\n";
    $d = $o->getTagDetail(1);
    echo "format 3: ", is_array($d) ? "{$d['width']}x{$d['height']}" : (var_export($d, true) . ' ' . limit_error()), "\n";
    $d = $o->getTagDetail(2);
    echo "alpha: ", isset($d['alpha_data_len']) ? $d['alpha_data_len'] : ('none ' . limit_error()), "\n";
    // the tags that were not parsed are written as they were read
    echo "output: ", var_export($o->output() === $data, true), "\n";
}
?>
===DONE===
--EXPECT--
SWF is smaller than 512K: true
== 512K: input=true
format 5: false limit
format 3: false limit
alpha: none limit
output: true
== 2M: input=true
format 5: false limit
format 3: 1000x1000
alpha: none limit
output: true
== 8M: input=true
format 5: 1000x1000
format 3: 1000x1000
alpha: 4194304
output: true
== -1: input=true
format 5: 1000x1000
format 3: 1000x1000
alpha: 4194304
output: true
===DONE===
