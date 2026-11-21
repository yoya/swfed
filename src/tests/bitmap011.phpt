--TEST--
replaceBitmapData() with a color condition does not match a JPEG bitmap of the given size (#155)
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
$jpeg = file_get_contents('tests/5.jpg');
$other = file_get_contents('tests/restart.jpg');
// 1: DefineBitsJPEG2, 2: DefineBitsLossless 2x2 of (200, 100, 50)
$data = swf(6, tag(21, pack('v', 1) . $jpeg)
             . tag(20, pack('vCvv', 2, 5, 2, 2) . gzcompress(str_repeat("\x00\xc8\x64\x32", 4)))
             . tag(1, '') . tag(0, ''));
$o = new SWFEditor();
$o->input($data);
$size = $o->getBitmapSize(1);
$w = $size['width'];
$h = $size['height'];

$original = array(1 => $o->getTagDataByCID(1), 2 => $o->getTagDataByCID(2));

function replace($cond) {
    global $data, $other, $original;
    $o = new SWFEditor();
    $o->input($data);
    $r = @$o->replaceBitmapData($cond, $other);
    $changed = array();
    foreach (array(1, 2) as $id) {
        if ($o->getTagDataByCID($id) !== $original[$id]) {
            $changed[] = $id;
        }
    }
    echo str_replace(array('"', ':'), array('', '='), json_encode($cond)),
         " => ", var_export($r, true), ", replaced: ", $changed ? implode(',', $changed) : 'none', "\n";
}

echo "JPEG is ", ($w > 2) ? 'larger than 2x2' : 'small', "\n";
replace(array('width' => $w, 'height' => $h));
replace(array('width' => $w, 'height' => $h, 'red' => 200));
replace(array('width' => $w, 'red' => 200, 'green' => 100, 'blue' => 50));
replace(array('height' => $h, 'blue' => 50));
replace(array('width' => 2, 'height' => 2, 'red' => 200));
replace(array('width' => 2, 'height' => 2, 'red' => 100));
replace(array('red' => 200, 'green' => 100, 'blue' => 50));
replace(array('width' => 2));
?>
===DONE===
--EXPECTF--
JPEG is larger than 2x2
{width=%d,height=%d} => true, replaced: 1
{width=%d,height=%d,red=200} => false, replaced: none
{width=%d,red=200,green=100,blue=50} => false, replaced: none
{height=%d,blue=50} => false, replaced: none
{width=2,height=2,red=200} => true, replaced: 2
{width=2,height=2,red=100} => false, replaced: none
{red=200,green=100,blue=50} => true, replaced: 2
{width=2} => true, replaced: 2
===DONE===
