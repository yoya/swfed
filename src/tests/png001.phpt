--TEST--
replacePNGData (transparent)
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--FILE--
<?php

$swf_filename = 'tests/saitama.swf';
$gif_filename = 'tests/negimiku001.png';

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

$obj->input($swfdata);
$image_id = 1;
$gifdata = file_get_contents($gif_filename);
$obj->replacePNGData($image_id, $gifdata);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
a762e0604ac899638ca1fff318a52ee3
===DONE===
