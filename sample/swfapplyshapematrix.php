<?php

if ($argc != 8) {
    fprintf(STDERR, "Usage: swfapplyshapematrix <swf_file> <shape_id> <scale_x> <scale_y> <rotate_rad> <trans_x> <trans_y>\n");
    exit(1);
}
list($progname, $swf_filename, $shape_id, $scale_x, $scale_y, $rotate_rad, $trans_x, $trans_y) = $argv;

$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

$result = $obj->applyShapeMatrixFactor($shape_id, $scale_x, $scale_y, $rotate_rad, $trans_x, $trans_y);

if ($result == false) {
    fprintf(STDERR, "applyShapeMatrixFactor($shape_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
