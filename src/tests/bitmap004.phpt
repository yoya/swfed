--TEST--
replaceBitmapData width
--FILE--
<?php

$swf_filename = 'tests/saitama.swf';
$png_filename = 'tests/negimiku001.png24';

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

$obj->input($swfdata);
$image_cond = array('width' => 108);
$pngdata = file_get_contents($png_filename);
$opts = array('rgb15' => true);
$obj->replaceBitmapData($image_cond, $pngdata, null, $opts);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
be5c46f5282d3280e3df44f1c5dd6906
===DONE===
