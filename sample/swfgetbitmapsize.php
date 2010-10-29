<?php

if ($argc < 3) {
    fprintf(STDERR, "Usage: swfgetbitmapsize.php <swf_file> <bitmap_id> [<bitmap_id2> [...]]\n");
    exit(1);
}

$bitmap_id_list = array_slice($argv, 2);

$swf = new SWFEditor();
$swf->input(file_get_contents($argv[1]));
foreach ($bitmap_id_list as $bitmap_id) {
    echo "bitmap_id:$bitmap_id";
    $size = $swf->getBitmapSize($bitmap_id);
    if ($size != false) {
        echo " width:${size['width']} height:${size['height']}";
    }
    echo PHP_EOL;
}
