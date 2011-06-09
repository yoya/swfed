<?php

if (($argc < 4) || ($argc%2 != 0)) {
    fprintf(STDERR, "Usage: swfreplaceactionstring <swf_file> <from_str> <to_str> [<from_str2> <to_str2> [...]]\n");
    exit(1);
}

$swf_filename = $argv[1];

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

$params = array();
for ($i=2 ; $i < $argc ; $i+=2) {
    $from_str = $argv[$i];
    $to_str = $argv[$i+1];
    $params[$from_str] = $to_str;
}

if ($obj->replaceActionString($params) == false) {
    fprintf(STDERR, "replaceActionString(params) failed\n");
    exit(1);
}


echo $obj->output();
