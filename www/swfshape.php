<?php

require_once('define.php');

$id       = $_REQUEST['id'];
$shape_id = $_REQUEST['shape_id'];
$ext      = $_REQUEST['ext'];

$shape_filename = "$tmp_prefix$id-$shape_id$ext";
$shape_data = file_get_contents($shape_filename);

$swf_data = file_get_contents("shapetpl.swf");

$swf = new SWFEditor();
$swf->input($swf_data);
$swf->replaceShapeData(1, $shape_data);

header("Content-type: application/x-shockwave-flash");
echo $swf->output();
