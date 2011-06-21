<?php

if ($argc < 4) {
    fprintf(STDERR, "Usage: swfreplacebitmapdata <swf_file> <image_id> <bitmap_file> [jpeg_alpha_file]\n");
    exit(1);
}

if ($argc > 3) {
    $swf_filename = $argv[1];
    $image_id = $argv[2];
    $bitmap_filename = $argv[3];
    if ($argc > 4) {
	$jpeg_alpha_filename = $argv[4];
    }
}

$swfdata = file_get_contents($swf_filename);
$bitmapdata = file_get_contents($bitmap_filename);

if (! empty($jpeg_alpha_filename)) {
    $jpeg_alphadata = file_get_contents($jpeg_alpha_filename);
}

$obj = new SWFEditor();

$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

if (empty($jpeg_alphadata)) {
    $result = $obj->replaceBitmapData($image_id, $bitmapdata);
} else {
    $result = $obj->replaceBitmapData($image_id, $bitmapdata, $jpeg_alphadata);
}

if ($result == false) {
    fprintf(STDERR, "replaceBitmapData($image_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
