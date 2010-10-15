<?php

if ($argc != 7) {
    fprintf(STDERR, "Usage: swfapplyshaperect <swf_file> <shape_id> <scale_x> <scale_y> <trans_x> <trans_y>\n");
    exit(1);
}
list($progname, $swf_filename, $shape_id, $scale_x, $scale_y, $trans_x, $trans_y) = $argv;

$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

$result = $obj->applyShapeRectFactor($shape_id, $scale_x, $scale_y, $trans_x, $trans_y);

if ($result == false) {
    fprintf(STDERR, "applyShapeRectFactor($shape_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
