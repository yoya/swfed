<?php

if ($argc < 4) {
    fprintf(STDERR, "Usage: swfreplacejpegdata <swf_file> <image_id> <jpeg_file> [alpha_file]\n");
    exit(1);
}

if ($argc > 3) {
    $swf_filename = $argv[1];
    $image_id = $argv[2];
    $jpeg_filename = $argv[3];
    if ($argc > 4) {
	$alpha_filename = $argv[4];
    }
}

$swfdata = file_get_contents($swf_filename);
$jpegdata = file_get_contents($jpeg_filename);

if (! empty($alpha_filename)) {
    $alphadata = file_get_contents($alpha_filename);
}

$obj = new SWFEditor();

$obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

if (empty($alphadata)) {
    $result = $obj->replaceJpegData($image_id, $jpegdata);
} else {
    $result = $obj->replaceJpegData($image_id, $jpegdata, $alphadata);
}

if ($result == false) {
    fprintf(STDERR, "replaceJpegdata($image_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
