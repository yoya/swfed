<?php

if ($argc < 3) {
    fprintf(STDERR, "Usage: swfgetshapeidlistbybitmapref.php <swf_file> <bitmap_id> [<bitmap_id2> [...]]\n");
    exit(1);
}

$bitmap_id_list = array_slice($argv, 2);

$swf = new SWFEditor();
$swf->input(file_get_contents($argv[1]));
foreach ($bitmap_id_list as $bitmap_id) {
    echo "$bitmap_id:";
    $shape_id_list = $swf->getShapeIdListByBitmapRef($bitmap_id);
    foreach ($shape_id_list as $shape_id) {
        echo " $shape_id";
    }
    echo PHP_EOL;
}
