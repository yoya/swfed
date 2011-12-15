<?php

require_once('define.php');

function multiply_unit($str) {
   $str = preg_replace('/(\d+)G/ei', "$1*1024*1024*1024", $str);
   $str = preg_replace('/(\d+)M/ei', "$1*1024*1024", $str);
   $str = preg_replace('/(\d+)K/ei', "$1*1024", $str);
   $str = preg_replace('/(\d+)B/ei', "$1", $str);
   return $str;
}
$upload_max_filesize  = ini_get('upload_max_filesize');
$upload_max_filesize_bytes = multiply_unit($upload_max_filesize);

function detect_image_ext(&$imagedata) {
    $image_sig = substr($imagedata, 0, 0x10);
    if (strpos($image_sig, 'JFIF') === 6) {
        $ext = '.jpg';
    } elseif (strpos($image_sig, 'PNG') === 1) {
        $ext = '.png';
    } elseif (strpos($image_sig, 'GIF') === 0) {
        $ext = '.gif';
    } else {
        $ext = false;
    }
    return $ext;
}

if (! empty($_FILES['imagefile']['tmp_name'])) {
    $filename = $_FILES['imagefile']['tmp_name'];
    $imagedata = file_get_contents($filename);
    if ($imagedata > $upload_max_filesize_bytes) {
        echo " $upload_max_filesize Bytes 以内のファイルしか受け付けません。\n";
        exit(0);
    }
    $tmp_name = sha1($imagedata, false);
    $id = $_REQUEST['id'];
    $image_id = $_REQUEST['image_id'];
    $ext = detect_image_ext($imagedata);
    if ($ext == false) {
        $image_sig = substr($imagedata, 0, 8);
        echo "unknown image signature: ".bin2hex($image_sig)."\n";
        exit(1);
    }
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
<body bgcolor="#fff0f0">
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
<body bgcolor="#fff0f0">
<form enctype="multipart/form-data" action="" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="$upload_max_filesize_bytes;

_bytes" />
    画像ファイルをアップロード: <input name="imagefile" type="file" />
    <input type="hidden" name="id" value="$id" />
    <input type="hidden" name="image_id" value="$image_id" />
    <input type="submit" name="action" value="replace" />
</form>
          ファイルを指定してください。($upload_max_filesize Bytes 以内に限定してます)
</body>
</html>
FORM;
    exit(0);
}

$id = $_REQUEST['id'];
$image_id = $_REQUEST['image_id'];
$id_image = $_REQUEST['id_image'];
$ext = $_REQUEST['ext'];

if (($ext != '.jpg') && ($ext != '.png') && ($ext != '.gif')) {
    echo "unknown ext=($ext)..\n";
    exit(1);
}

$swf_filename = "$tmp_prefix$id.swf";
$swfdata = file_get_contents($swf_filename);
$image_filename = "$tmp_prefix$id_image$ext";
$imagedata = file_get_contents($image_filename);

$swf = new SWFEditor();
$swf->setShapeAdjustMode(SWFEditor::SHAPE_BITMAP_RECT_RESIZE);

if ($swf->input($swfdata) == false) {
    echo "input failed\n";
    exit(1);
}

switch ($ext) {
  case '.jpg':
    $result = $swf->replaceJpegData(intval($image_id), $imagedata);
    break;
  case '.png':
    $result = $swf->replacePNGData(intval($image_id), $imagedata);
    break;
  case '.gif':
    $result = $swf->replaceGIFData(intval($image_id), $imagedata);
    break;
  default:
    echo "unknown ext($ext)...\n";
    exit(1);
}

if ($result == false) {
    echo "replace failed ext=$ext\n";
    exit(0);
}

header('Content-type: application/x-shockwave-flash');
// header('Content-type: application/octet-stream');
echo $swf->output();
