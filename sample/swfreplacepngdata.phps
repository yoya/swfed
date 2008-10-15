<?php

if (($argc < 4) || ($argc%2 != 0)) {
	fprintf(STDERR, "Usage: swfreplacepngdata <swf_file> <image_id> <png_file> [<image_id2> <png_file2> [...]]\n");
	exit(1);
}

$swf_filename = $argv[1];

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
	fprintf(STDERR, "input failed\n");
	exit (1);
}

for ($i=2 ; $i < $argc ; $i+=2) {
	$image_id = $argv[$i];
	$png_filename = $argv[$i+1];
	$pngdata = file_get_contents($png_filename);
	if ($obj->replacePNGData($image_id, $pngdata) == false) {
		fprintf(STDERR, "replacePNGdata($image_id, ...) failed\n");
		exit (1);
	}
}

echo $obj->output();
