--TEST--
replaceBitmapData width
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
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
7c045523b14b62d31ea5875cea5bfd59
===DONE===
