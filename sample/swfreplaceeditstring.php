<?php

if ($argc < 4) {
    fprintf(STDERR, "Usage: swfreplaceeditstring <swf_file> <variable_name> <initial_text>\n");
    exit(1);
}

$swf_filename = $argv[1];
$variable_name = $argv[2];
$initial_text = $argv[3];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}
if ($obj->replaceEditString($variable_name, $initial_text) == false) {
    fprintf(STDERR, "replaceEditString($variable_name, ...) failed\n");
    exit(1);
}

echo $obj->output();
