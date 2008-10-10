<?php

if ($argc != 3) {
    fprintf(STDERR, "Usage: swfgetpngdata <swf_file> <image_id>\n");
    exit(1);
}

$swf_filename = $argv[1];
$image_id = $argv[2];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
$obj->input($swfdata);

echo $obj->getPNGData($image_id);
