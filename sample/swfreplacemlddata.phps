<?php

if (($argc < 4) || ($argc%2 != 0)) {
    fprintf(STDERR, "Usage: swfreplacemlddata <swf_file> <sound_id> <mld_file> [<sound_id2> <mld_file2> [...]]\n");
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
    $sound_id = $argv[$i];
    $mld_filename = $argv[$i+1];
    $mlddata = file_get_contents($mld_filename);
    if ($obj->replaceMLDData($sound_id, $mlddata) == false) {
	fprintf(STDERR, "replaceMLDdata($sound_id, ...) failed\n");
        exit(1);
    }
}

echo $obj->output();
