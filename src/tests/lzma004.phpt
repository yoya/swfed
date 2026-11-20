--TEST--
LZMA compressed SWF (ZWS): swfed.max_uncompressed_size and a file_length that does not match the data
--SKIPIF--
<?php
ob_start();
phpinfo(INFO_MODULES);
if (strpos(ob_get_clean(), 'LZMA (ZWS) support => yes') === false) {
    die('skip swfed built without liblzma');
}
?>
--FILE--
<?php
// one unknown tag of 4MB zeros: the ZWS is tiny but 4MB after decompression
$size = 4 * 1024 * 1024;
$body = "\x00" . pack('vv', 12 << 8, 1) . pack('vV', (255 << 6) | 0x3f, $size) .
        str_repeat("\0", $size) . pack('v', 0);
$o = new SWFEditor();
var_dump($o->input("FWS\x0A" . pack('V', 8 + strlen($body)) . $body));
$o->setHeaderInfo(array('lzma' => true));
$zws = $o->output();
echo substr($zws, 0, 3), " ", strlen($zws) < 4096 ? "small" : "large", "\n";
$length = unpack('V', substr($zws, 4, 4))[1]; // file_length: the size after decompression
var_dump($length > $size);

// the limit is the size after decompression (file_length), not the size of the ZWS
$n = new SWFEditor();
ini_set('swfed.max_uncompressed_size', '1M');
var_dump($n->input($zws));
var_dump($n->replaceMovieClip('x', $zws));
ini_set('swfed.max_uncompressed_size', '8M');
var_dump($n->input($zws), count($n->getTagList()));
ini_set('swfed.max_uncompressed_size', (string) $length);
var_dump($n->input($zws));
ini_set('swfed.max_uncompressed_size', (string) ($length - 1));
var_dump($n->input($zws));
ini_set('swfed.max_uncompressed_size', '-1');
var_dump($n->input($zws));

// file_length is not trusted: a value larger than the data makes is a failure
// (the buffer grows with the decompressed data, it is not allocated up front)
ini_set('swfed.max_uncompressed_size', '16M');
$bigger = substr_replace($zws, pack('V', 8 * 1024 * 1024), 4, 4);
var_dump($n->input($bigger));
// and a smaller one cuts the movie short in the middle of the tag: a failure too.
// The movie loaded before is kept.
$smaller = substr_replace($zws, pack('V', 1024 * 1024), 4, 4);
var_dump($n->input($smaller));
var_dump(count($n->getTagList()));
?>
===DONE===
--EXPECTF--
bool(true)
ZWS small
bool(true)

Warning: SWFEditor::input(): SWF file_length(%d) exceeds the limit(1048576) of swfed.max_uncompressed_size in %s on line %d
bool(false)

Warning: SWFEditor::replaceMovieClip(): SWF file_length(%d) exceeds the limit(1048576) of swfed.max_uncompressed_size in %s on line %d
bool(false)
bool(true)
int(2)
bool(true)

Warning: SWFEditor::input(): SWF file_length(%d) exceeds the limit(%d) of swfed.max_uncompressed_size in %s on line %d
bool(false)
bool(true)
%Abool(false)
%Abool(false)
int(2)
===DONE===
