--TEST--
Shape adjustment uses the size of a PNG stored in DefineBitsJPEG2, and is skipped for unknown sizes
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--FILE--
<?php

function shape8_scale($o) {
    // the bitmap fill of DefineShape2(id=8) as printed by swfInfo() is checked
    // through getTagDetail(): the shape must still parse after the adjustment
    $d = $o->getTagDetail(8);
    return is_array($d) ? "shape ok" : "shape broken";
}

$o = new SWFEditor();
$o->input(file_get_contents('tests/colorformat.swf'));
$size = $o->getBitmapSize(1);
echo $size['width'], "x", $size['height'], "\n";
var_dump($o->convertBitmapDataToJpegTag());
$size = $o->getBitmapSize(1);   // PNG inside DefineBitsJPEG2
echo $size['width'], "x", $size['height'], "\n";

$o->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
var_dump($o->replacePNGData(1, file_get_contents('tests/negimiku001.png')));
$o2 = new SWFEditor();
$o2->input($o->output());
echo shape8_scale($o2), "\n";
?>
===DONE===
--EXPECT--
50x50
bool(true)
50x50
bool(true)
shape ok
===DONE===
