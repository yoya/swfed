--TEST--
output() keeps the long tag header of a short tag: an unedited SWF is written as it was read (#158)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function short_tag($code, $body) {
    return pack('v', ($code << 6) | strlen($body)) . $body;
}
function long_tag($code, $body) {
    return pack('vV', ($code << 6) | 0x3f, strlen($body)) . $body;
}
function swf($tags) {
    // FrameSize 550x400 pixels, FrameRate 12, FrameCount 1
    $body = "\x78\x00\x05\x5f\x00\x00\x0f\xa0\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr(6) . pack('V', 8 + strlen($body)) . $body;
}
function push($s) {
    $d = "\x00" . $s . "\0";
    return "\x96" . pack('v', strlen($d)) . $d;
}
function headers($swf) { // the form of every top level tag header
    $r = array();
    for ($p = 8 + 9 + 4; $p + 2 <= strlen($swf); ) {
        $h = unpack('v', substr($swf, $p, 2))[1];
        $len = $h & 0x3f;
        $p += 2;
        if ($len == 0x3f) {
            $len = unpack('V', substr($swf, $p, 4))[1];
            $p += 4;
            $r[] = ($h >> 6) . ':long';
        } else {
            $r[] = ($h >> 6) . ':short';
        }
        $p += $len;
    }
    return implode(' ', $r);
}

$action = push('a') . "\x00";
$data = swf(long_tag(9, "\xff\xff\xff") . short_tag(9, "\x00\x00\x00")           // SetBackgroundColor
          . long_tag(12, $action) . short_tag(12, $action)                       // DoAction
          . long_tag(39, pack('vv', 1, 1) . long_tag(12, $action) . long_tag(1, '') . short_tag(0, '')) // DefineSprite
          . long_tag(1, '') . long_tag(0, ''));
$o = new SWFEditor();
var_dump($o->input($data));
echo "as read:   ", headers($data), "\n";
echo "unedited:  ", var_export($o->output() === $data, true), "\n";

// edited tags keep their form, a new tag is short
var_dump($o->replaceActionStrings(array('a' => 'bb')));
echo "edited:    ", headers($o->output()), "\n";
var_dump($o->rebuild());
echo "rebuilt:   ", headers($o->output()), "\n";
$o2 = new SWFEditor();
$o2->input(swf(long_tag(1, '') . long_tag(0, '')));
var_dump($o2->setActionVariables(array('k' => 'v')));
echo "new tag:   ", headers($o2->output()), "\n";

// 63 bytes and over, and the bitmap tags, are long whatever they were
$o3 = new SWFEditor();
$o3->input(swf(short_tag(20, pack('vCvv', 1, 5, 1, 1) . gzcompress("\x00\x01\x02\x03")) . short_tag(12, push(str_repeat('x', 40)) . "\x00")
             . short_tag(1, '') . short_tag(0, '')));
var_dump($o3->replaceActionStrings(array(str_repeat('x', 40) => str_repeat('y', 70))));
echo "forced:    ", headers($o3->output()), "\n";

// the SWF files of the tests, uncompressed
foreach (array('saitama.swf', 'negimiku.swf', 'colorformat.swf', 'textvar.swf') as $file) {
    $o = new SWFEditor();
    $o->input(file_get_contents("tests/$file"));
    $o->setHeaderInfo(array('compress' => false));
    $plain = $o->output();
    $o = new SWFEditor();
    $o->input($plain);
    echo "$file: ", var_export($o->output() === $plain, true), "\n";
}
?>
===DONE===
--EXPECT--
bool(true)
as read:   9:long 9:short 12:long 12:short 39:long 1:long 0:long
unedited:  true
bool(true)
edited:    9:long 9:short 12:long 12:short 39:long 1:long 0:long
bool(true)
rebuilt:   9:long 9:short 12:long 12:short 39:long 1:long 0:long
bool(true)
new tag:   12:short 1:long 0:long
bool(true)
forced:    20:long 12:long 1:short 0:short
saitama.swf: true
negimiku.swf: true
colorformat.swf: true
textvar.swf: true
===DONE===
