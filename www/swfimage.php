<?php

require_once('define.php');

$id       = $_REQUEST['id'];
$image_id = $_REQUEST['image_id'];
$ext      = $_REQUEST['ext'];

$image_filename = "$tmp_prefix$id-$image_id$ext";
$image_data = file_get_contents($image_filename);

if ($ext == '.jpg') {
    header("Content-type: image/jpeg");
} else {
    header("Content-type: image/png");
}

echo $image_data;
