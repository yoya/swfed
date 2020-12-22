<?php

require_once('define.php');

$id       = hex_from_string($_REQUEST['id']);
$shape_id = dec_from_string($_REQUEST['shape_id']);
$ext      = $_REQUEST['ext'];
if (($ext != '.jpg') && ($ext != '.png') && ($ext != '.gif')) {
    echo "unknown ext=($ext)..\n";
    exit(1);
}

$shape_filename = "$tmp_prefix$id-$shape_id$ext";
$shape_data = file_get_contents($shape_filename);

$swf_data = file_get_contents("shapetpl.swf");

$swf = new SWFEditor();
$swf->input($swf_data);
if ($swf->replaceShapeData(1, $shape_data) === false) {
    fprintf(STDERR, "replaceShapeData(1, ...) failed. id=$id\n");
}

header("Content-type: application/x-shockwave-flash");
$header_info = array(
	'x_min' => -100, 'y_min' => -100
);
$swf->setHeaderInfo($header_info);
echo $swf->output();
