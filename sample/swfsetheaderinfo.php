<?php

if (($argc < 4) || ($argc%2 != 0)) {
	fprintf(STDERR, "Usage: swfsetheaderinfo <swf_file> <key> <value> [<key2> <value2> [...]]\n");
	exit(1);
}

$swf_filename = $argv[1];

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

if ($obj->input($swfdata) == false) {
	fprintf(STDERR, "input failed\n");
	exit(1);
}

$info = $obj->getHeaderInfo();

for ($i=2 ; $i < $argc ; $i+=2) {
	$info[$argv[$i]] = $argv[$i+1];
}

if ($obj->setHeaderInfo($info) == false) {
	fprintf(STDERR, "setHeaderInfo(Array) failed\n");
	exit(1);
}

echo $obj->output();
