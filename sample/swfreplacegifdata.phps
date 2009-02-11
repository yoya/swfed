<?php

if (($argc < 4) || ($argc%2 != 0)) {
    fprintf(STDERR, "Usage: swfreplacegifdata <swf_file> <image_id> <gif_file> [<image_id2> <gif_file2> [...]]\n");
    exit(1);
}

$swf_filename = $argv[1];

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

for ($i=2 ; $i < $argc ; $i+=2) {
    $image_id = $argv[$i];
    $gif_filename = $argv[$i+1];
    $gifdata = file_get_contents($gif_filename);
    if ($obj->replaceGIFData($image_id, $gifdata) == false) {
        fprintf(STDERR, "replaceGIFdata($image_id, ...) failed\n");
        exit(1);
    }
}

echo $obj->output();
