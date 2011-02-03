<?php

if ($argc != 3) {
    fprintf(STDERR, "Usage: swfgetshapedata <swf_file> <shape_id>\n");
    exit(1);
}

$swf_filename = $argv[1];
$shape_id = $argv[2];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

echo $obj->getShapeData($shape_id);



