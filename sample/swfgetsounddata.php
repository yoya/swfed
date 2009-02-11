<?php

if ($argc != 3) {
    fprintf(STDERR, "Usage: swfgetsounddata <swf_file> <sound_id>\n");
    exit(1);
}

$swf_filename = $argv[1];
$sound_id = $argv[2];
$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}

echo $obj->getSoundData($sound_id);
