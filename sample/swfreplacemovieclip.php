<?php

if (($argc < 4) || ($argc%2 != 0)) {
	fprintf(STDERR, "Usage: swfreplacemovieclip <swf_file> <instance_name> <movieclip_swffile> [<instance_name2> <movieclip_swffile2> [...]]\n");
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
	$instance_name = $argv[$i];
	$movieclip_swffilename = $argv[$i+1];
	$movieclip_swfdata = file_get_contents($movieclip_swffilename);
	if ($obj->replaceMovieClip($instance_name, $movieclip_swfdata) == false) {
		fprintf(STDERR, "replaceMovieClip($instance_name, ...) failed\n");
		exit(1);
	}
}

echo $obj->output();
