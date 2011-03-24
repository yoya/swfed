<?php

if ($argc != 2) {
	fprintf(STDERR, "Usage: swfgetheaderinfo <swf_file>\n");
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

foreach ($info as $key => $value) {
	if (is_bool($value)) {
	   if ($value === true) {
	      $value = 'true';
	   } else {
	      $value = 'false';
	   }
	}
	echo "$key: $value\n";
}
