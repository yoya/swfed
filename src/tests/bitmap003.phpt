--TEST--
replaceBitmapData width
--FILE--
<?php

$swf_filename = 'tests/saitama.swf';
$png_filename = 'tests/negimiku001.png';

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

$obj->input($swfdata);
$image_cond = array('width' => 108);
$pngdata = file_get_contents($png_filename);
$obj->replaceBitmapData($image_cond, $pngdata);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
460b54df088502aed74b8042c09fcfef
===DONE===
