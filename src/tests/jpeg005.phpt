--TEST--
DefineBitsJPEG4 is handled as a bitmap
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
$jpeg = file_get_contents('tests/5.jpg');            // 400x300
$alpha = str_repeat("\x80", 400 * 300);
// CharacterID, AlphaDataOffset, DeblockParam (8.8 fixed: 1.0), ImageData, BitmapAlphaData
$jpeg4 = tag(90, pack('vVv', 1, strlen($jpeg), 0x0100) . $jpeg . gzcompress($alpha));
$swf = swf(10, tag(69, pack('V', 0)) . tag(9, "\xff\xff\xff") . $jpeg4
                . tag(26, "\x02" . pack('vv', 1, 1)) . tag(1, '') . tag(0, ''));

$o = new SWFEditor();
$o->input($swf);
$list = $o->getTagList();
echo $list[2]['tagName'], "\n";
var_dump($list[2]['detail']);
var_dump(strlen($o->getJpegData(1)) > 0);
var_dump($o->getJpegAlpha(1) === $alpha);
$size = $o->getBitmapSize(1);
echo $size['width'], "x", $size['height'], "\n";
$d = $o->getTagDetail(2);
echo "image_id=", $d['image_id'], " alpha_data_len=", $d['alpha_data_len'], "\n";

// rebuild keeps DeblockParam
var_dump($o->rebuild());
$data = $o->getTagData(2);
echo "deblock=", bin2hex(substr($data, 6 + 6, 2)), "\n";

// replacement keeps DefineBitsJPEG4 and DeblockParam
$alpha2 = str_repeat("\x40", 400 * 300);
var_dump($o->replaceJpegData(1, $jpeg, $alpha2));
$list = $o->getTagList();
echo $list[2]['tagName'], "\n";
var_dump($o->getJpegAlpha(1) === $alpha2);
var_dump($o->rebuild());
echo "deblock=", bin2hex(substr($o->getTagData(2), 6 + 6, 2)), "\n";

// a tag that is not parsed yet (right after input()) keeps DeblockParam too
$o->input($swf);
var_dump($o->replaceJpegData(1, $jpeg, $alpha2));
$list = $o->getTagList();
echo $list[2]['tagName'], "\n";
var_dump($o->getJpegAlpha(1) === $alpha2);
var_dump($o->rebuild());
echo "deblock=", bin2hex(substr($o->getTagData(2), 6 + 6, 2)), "\n";

// without alpha it becomes DefineBitsJPEG2 (as DefineBitsJPEG3 does),
// whether the tag is parsed or not, and the old alpha does not stay
foreach (array('not parsed', 'parsed') as $state) {
    $o->input($swf);
    if ($state == 'parsed') {
        $o->rebuild();
    }
    var_dump($o->replaceJpegData(1, $jpeg));
    $list = $o->getTagList();
    echo $state, ": ", $list[2]['tagName'], "\n";
    var_dump($o->getJpegAlpha(1));
    $o2 = new SWFEditor();
    var_dump($o2->input($o->output()));
    $list = $o2->getTagList();
    echo $list[2]['tagName'], "\n";
    var_dump($o2->getJpegAlpha(1));
}

var_dump($o->isBitmapTagData($jpeg4));

// a rebuilt DefineBitsJPEG4 has the long form tag header even if it is short
$o->input(swf(10, tag(90, pack('vVv', 1, 4, 0) . "\xff\xd8\xff\xd9" . gzcompress("\x00")) . tag(0, '')));
var_dump($o->rebuild());
echo bin2hex(substr($o->getTagData(0), 0, 2)), "\n";

// replaceMovieClip moves it into the parent SWF
$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->replaceMovieClip('miku', $swf));
$codes = array_map(function ($t) { return $t['code']; }, $o->getTagList());
var_dump(in_array(90, $codes));
?>
===DONE===
--EXPECT--
DefineBitsJPEG4
bool(true)
bool(true)
bool(true)
400x300
image_id=1 alpha_data_len=120000
bool(true)
deblock=0001
bool(true)
DefineBitsJPEG4
bool(true)
bool(true)
deblock=0001
bool(true)
DefineBitsJPEG4
bool(true)
bool(true)
deblock=0001
bool(true)
not parsed: DefineBitsJPEG2
bool(false)
bool(true)
DefineBitsJPEG2
bool(false)
bool(true)
parsed: DefineBitsJPEG2
bool(false)
bool(true)
DefineBitsJPEG2
bool(false)
bool(true)
bool(true)
bf16
bool(true)
bool(true)
===DONE===
