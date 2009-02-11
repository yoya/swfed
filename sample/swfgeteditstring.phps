<?php

if ($argc != 3) {
    fprintf(STDERR, "Usage: swfgeteditstrint <swf_file> <var_name>\n");
    exit(1);
}

$swf_filename = $argv[1];
$var_name = $argv[2];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

echo $obj->getEditString($var_name) . "\n";
