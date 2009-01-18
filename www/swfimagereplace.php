<?php

require_once('define.php');

if (! empty($_FILES['imagefile']['tmp_name'])) {
    $filename = $_FILES['imagefile']['tmp_name'];
    $imagedata = file_get_contents($filename);
    if ($imagedata > 67108864) {
        echo ' 64M Bytes 以内のファイルしか受け付けません。'."\n";
        exit(0);
    }
    $tmp_name = sha1($imagedata, false);
    $id = $_REQUEST['id'];
    $image_id = $_REQUEST['image_id'];
    $ext = $_REQUEST['ext'];
    $id_image = substr($tmp_name, 0, 16); // XXX
    $tmp_filename = "$tmp_prefix$id_image$ext";
    if ((! is_readable($tmp_filename)) &&
        (! file_put_contents($tmp_filename, $imagedata))) {
        fprintf(stderr, "swfimagereplace.php: file_put_contents failed. zero size?\n");
        unlink($tmp_filename);
        exit(0);
    }
//    header("Location: ./swfimagereplace.php?id=$id&image_id=$image_id&id_image=$id_image&ext=$ext");
echo <<< FORM
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
</head>
<body>
<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0" width="100%" height="100%">
<param name="movie" value="./swfimagereplace.php?id=$id&image_id=$image_id&id_image=$id_image&ext=$ext">
<param name="quality" value="high">
</object>
</body>
</html>
FORM;
    exit(0);
}

if (empty($_REQUEST['id_image']))  {
    $id       = $_REQUEST['id'];
    $image_id = $_REQUEST['image_id'];
echo <<< FORM
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
</head>
<body>
<form enctype="multipart/form-data" action="" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="67108864" />
    画像ファイルをアップロード: <input name="imagefile" type="file" />
    <input type="hidden" name="id" value="$id" />
    <input type="hidden" name="image_id" value="$image_id" />
    <input type="submit" name="ext" value=".png" />
    <input type="submit" name="ext" value=".jpg" />
</form>
          ファイルを指定してください。(64MBytes 以内に限定してます)
</body>
</html>
FORM;
    exit(0);
}

$id = $_REQUEST['id'];
$image_id = $_REQUEST['image_id'];
$id_image = $_REQUEST['id_image'];
$ext = $_REQUEST['ext'];
if (($ext != '.png') && ($ext != '.jpg')) {
    exit(1);
}
$swf_filename = "$tmp_prefix$id.swf";
$swfdata = file_get_contents($swf_filename);
$image_filename = "$tmp_prefix$id_image$ext";
$imagedata = file_get_contents($image_filename);

$swf = new SWFEditor();
$swf->input($swfdata);

if ($ext == '.jpg') {
    $swf->replaceJpegData(intval($image_id), $imagedata);
} else {
    $swf->replacePNGData(intval($image_id), $imagedata);
}

header('Content-type: application/x-shockwave-flash');
// header('Content-type: application/octet-stream');
echo $swf->output();
