<?php

if (($argc != 3) && ($argc != 4)) {
    fprintf(STDERR, "Usage: swfremovetag <swf_file> <tag_seqno> [<tag_seqno_in_sprite>]\n");
    exit(1);
}

$swf_filename = $argv[1];

$swfdata = file_get_contents($swf_filename);
$obj = new SWFEditor();

if ($obj->input($swfdata) == false) {
    fprintf(STDERR, "input failed\n");
    exit(1);
}
$tag_seqno = $argv[2];

if ($argc == 3) {
    if ($obj->removeTag($tag_seqno) == false) {
        fprintf(STDERR, "removeTag($tag_seqno) failed\n");
        exit(1);
    }
} else {
    $tag_seqno_in_sprite = $argv[3];
    if ($obj->removeTag($tag_seqno, $tag_seqno_in_sprite) == false) {
        fprintf(STDERR, "removeTag($tag_seqno, $tag_seqno_in_sprite) failed\n");
        exit(1);
    }
}

echo $obj->output();
