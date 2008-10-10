<?php

if ($argc < 2) {
    fprintf(STDERR, "Usage: swfdump <swf_file>\n");
    exit(1);
}

$swf_filename = $argv[1];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
$obj->input($swfdata);
$obj->swfInfo();
