<?php

if (($argc < 4) || ($argc%2 != 0)) {
	fprintf(STDERR, "Usage: swfreplaceshapedata <swf_file> <shape_id> <shape_file> [<shape_id2> <shape_file2> [...]]\n");
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
	$shape_id = $argv[$i];
	$shape_filename = $argv[$i+1];
	$shapedata = file_get_contents($shape_filename);
	if ($obj->replaceShapeData($shape_id, $shapedata) == false) {
		fprintf(STDERR, "replaceShapeData($shape_id, ...) failed\n");
		exit(1);
	}
}

echo $obj->output();
