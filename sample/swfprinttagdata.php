<?php

if ($argc != 2) {
    fprintf(STDERR, "Usage: swfprinttag <tagdata>\n");
    exit(1);
}

$tagdata_filename = $argv[1];

$tagdata = file_get_contents($tagdata_filename);

$obj = new SWFEditor();

if ($obj->printTagData($tagdata) == false) {
    fprintf(STDERR, "printTagData(...) failed\n");
    exit(1);
}

exit (0);

