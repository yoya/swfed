<?php

if ($argc < 3) {
    fprintf(STDERR, "Usage: swfsetactionvariables <swf_file> <key=value> [<key2=value2> [...]]\n");
    exit(1);
}

$swf_filename = $argv[1];
$key_value_list = array_splice($argv, 2);
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
	fprintf(STDERR, "input failed\n");
	exit(1);
}

$variables = array();

foreach ($key_value_list as $key_value) {
    list($key, $value) = explode('=', $key_value, 2);
    $variables[$key] = $value;
}

if ($obj->setActionVariables($variables) == false) {
    echo "failed\n";
    exit(1);
}

echo $obj->output();
