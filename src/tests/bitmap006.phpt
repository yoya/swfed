--TEST--
replaceBitmapData select bitmap from defineshape fillstyles
--FILE--
<?php

$swf_filename = 'tests/colorformat.swf';
$png_filename = 'tests/negimiku001.png';

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();


// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

$obj->input($swfdata);
$image_cond = 6;
$pngdata = file_get_contents($png_filename);
$obj->replaceBitmapData($image_cond, $pngdata);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
859168c1f4db41f75d84efdbe63dd785
===DONE===
