--TEST--
getJpegData(), getJpegAlpha(), getPNGData(), getShapeIdListByBitmapRef(), convertBitmapDataToJpegTag()
--FILE--
<?php
function size($image) {
    $s = getimagesizefromstring($image);
    return $s[0] . 'x' . $s[1];
}
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));

echo "* getJpegData\n";
$jpeg = $o->getJpegData(1); // DefineBitsJPEG
echo bin2hex(substr($jpeg, 0, 2)), " ", size($jpeg), "\n";
$jpeg = $o->getJpegData(3); // DefineBitsJPEG3
echo bin2hex(substr($jpeg, 0, 2)), " ", size($jpeg), "\n";
var_dump($o->getJpegData(2)); // DefineShape

echo "* getJpegAlpha\n";
$size = $o->getBitmapSize(3);
var_dump(strlen($o->getJpegAlpha(3)) === $size['width'] * $size['height']);
var_dump($o->getJpegAlpha(1)); // no alpha

echo "* getShapeIdListByBitmapRef\n";
echo json_encode($o->getShapeIdListByBitmapRef(1)), json_encode($o->getShapeIdListByBitmapRef(5)),
     json_encode($o->getShapeIdListByBitmapRef(99)), "\n";

echo "* getPNGData\n";
$o->input(file_get_contents('tests/colorformat.swf')); // DefineBitsLossless / 2
foreach (array(1, 2, 3, 5, 6, 7) as $id) {
    $png = $o->getPNGData($id);
    $size = $o->getBitmapSize($id);
    echo $id, ": ", bin2hex(substr($png, 0, 8)), " ", size($png),
         ($size['width'] . 'x' . $size['height'] === size($png)) ? " ok" : " NG", "\n";
}
var_dump($o->getPNGData(4));

echo "* convertBitmapDataToJpegTag\n";
var_dump($o->convertBitmapDataToJpegTag());
echo implode(',', array_map(function ($t) { return $t['code']; }, $o->getTagList())), "\n";
$o2 = new SWFEditor();
$o2->input($o->output());
echo bin2hex(substr($o2->getTagContentsByCID(1), 0, 8)), "\n"; // PNG in DefineBitsJPEG2
?>
===DONE===
--EXPECTF--
* getJpegData
ffd8 320x240
ffd8 219x238
%Abool(false)
* getJpegAlpha
bool(true)
%Abool(false)
* getShapeIdListByBitmapRef
[2][6][]
* getPNGData
1: 89504e470d0a1a0a 50x50 ok
2: 89504e470d0a1a0a 50x100 ok
3: 89504e470d0a1a0a 100x50 ok
5: 89504e470d0a1a0a 50x50 ok
6: 89504e470d0a1a0a 100x100 ok
7: 89504e470d0a1a0a 100x100 ok
%Abool(false)
* convertBitmapDataToJpegTag
bool(true)
9,21,21,21,21,21,21,21,22,26,1,0
89504e470d0a1a0a
===DONE===
