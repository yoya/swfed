<?php

if ($argc < 5) {
    fprintf(STDERR, "Usage: swfreplacebitmapdata2 <swf_file> <image_id> <bitmap_file> <opts[:opts2[:...]]>\n");
    fprintf(STDERR, "ex) swfreplacebitmapdata2 test.swf 1 test.png without_converting\n");
    fprintf(STDERR, "ex) swfreplacebitmapdata2 test.swf 1 test.png rgb15\n");
    exit(1);
}

if ($argc > 4) {
    $swf_filename = $argv[1];
    $image_id = $argv[2];
    $bitmap_filename = $argv[3];
    $opts = array();
    foreach (explode(':', $argv[4]) as $key) {
        $opts[$key] = true;
    }
}

$swfdata = file_get_contents($swf_filename);
$bitmapdata = file_get_contents($bitmap_filename);

$obj = new SWFEditor();

 $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE);
// $obj->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_TYPE_TILLED);

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

if ((count($opts) === 1) && (! empty($opts['without_converting']))) {
    $result = $obj->replaceBitmapData($image_id, $bitmapdata, null, true);
} else {
    $result = $obj->replaceBitmapData($image_id, $bitmapdata, null, $opts);
}

if ($result == false) {
    fprintf(STDERR, "replaceBitmapData($image_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
