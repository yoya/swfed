--TEST--
swfed.max_uncompressed_size limits the size after decompression
--INI--
memory_limit=16M
--FILE--
<?php

// CWS whose single tag is 100MB of zeros (about 100KB compressed)
function big_cws() {
    $size = 100 * 1024 * 1024;
    $head = "\x00" . pack('vv', 12 << 8, 1) . pack('vV', (255 << 6) | 0x3f, $size);
    $tail = pack('v', 0);
    $ctx = deflate_init(ZLIB_ENCODING_DEFLATE);
    $z = deflate_add($ctx, $head, ZLIB_NO_FLUSH);
    $chunk = str_repeat("\0", 1024 * 1024);
    for ($i = 0; $i < 100; $i++) {
        $z .= deflate_add($ctx, $chunk, ZLIB_NO_FLUSH);
    }
    $z .= deflate_add($ctx, $tail, ZLIB_FINISH);
    $length = 8 + strlen($head) + $size + strlen($tail);
    return "CWS\x0A" . pack('V', $length) . $z;
}

$cws = big_cws();
echo strlen($cws) < 200000 ? "compressed small\n" : "compressed large\n";

// default (0): memory_limit (16M) is the limit
$o = new SWFEditor();
var_dump($o->input($cws));

$saitama = file_get_contents('tests/saitama.swf'); // file_length=2704
ini_set('swfed.max_uncompressed_size', '1K');
var_dump($o->input($saitama));
var_dump($o->replaceMovieClip('miku', $saitama));
ini_set('swfed.max_uncompressed_size', '4K');
var_dump($o->input($saitama));
ini_set('swfed.max_uncompressed_size', '-1');
var_dump($o->input($saitama));
?>
===DONE===
--EXPECTF--
compressed small

Warning: SWFEditor::input(): SWF file_length(%d) exceeds the limit(16777216) of swfed.max_uncompressed_size in %s on line %d
bool(false)

Warning: SWFEditor::input(): SWF file_length(2704) exceeds the limit(1024) of swfed.max_uncompressed_size in %s on line %d
bool(false)

Warning: SWFEditor::replaceMovieClip(): SWF file_length(2704) exceeds the limit(1024) of swfed.max_uncompressed_size in %s on line %d
bool(false)
bool(true)
bool(true)
===DONE===
