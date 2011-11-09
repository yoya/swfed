<?php

if ($argc != 2) {
    fprintf(STDERR, "Usage: swfconvertbitmapdatatojpegtag.php <swf_file>\n");
    exit(1);
}

$swf_filename = $argv[1];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

$ret = $obj->convertBitmapDataToJpegTag();

if ($ret == false) {
    fprintf(STDERR, "replaceBitmapData($image_id, ...) failed\n");
    exit(1);
}

echo $obj->output();
