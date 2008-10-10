<?php

$compress = true;

switch($argc) {
    case 2:
	$swf_filename = $argv[1];
        break;
    case 3:
	if ($argv[1] == '-u') {
	    $compress = false;
	    $swf_filename = $argv[2];
	    break;
	}
    default:
	fprintf(STDERR, "Usage: php swfcompress.phps [-u] <swf_file>\n");
	exit(1);
}

$swf_data = file_get_contents($swf_filename);

$swf = new SWFEditor();
$swf->input($swf_data);
$info = array('compress' => $compress);
$swf->setHeaderInfo($info);

echo $swf->output();
