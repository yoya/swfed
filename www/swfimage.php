<?php

require_once('define.php');

$id       = $_REQUEST['id'];
$image_id = $_REQUEST['image_id'];
$ext      = $_REQUEST['ext'];

switch ($ext) {
  case '.jpg':
    header("Content-type: image/jpeg");
    break;
  case '.png':
    header("Content-type: image/png");
    break;
  default:
    exit(1);
}

$image_filename = "$tmp_prefix$id-$image_id$ext";
$image_data = file_get_contents($image_filename);
echo $image_data;
