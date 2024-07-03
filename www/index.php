<?php

require_once('define.php');

function multiply_unit($str) {
   $str = preg_replace_callback('/(\d+)G/i', function($m) {
           return $m[1]*1024*1024*1024; }, $str);
   $str = preg_replace_callback('/(\d+)M/i', function($m) {
           return $m[1]*1024*1024; }, $str);
   $str = preg_replace_callback('/(\d+)K/i', function($m) {
           return $m[1]*1024; }, $str);
   $str = preg_replace_callback('/(\d+)B/i', function($m) {
           return $m[1]; } , $str);
   return $str;
}
$upload_max_filesize  = ini_get('upload_max_filesize');
$upload_max_filesize_bytes = multiply_unit($upload_max_filesize);

if (isset($_FILES['swffile']['tmp_name'])) {
    $filename = $_FILES['swffile']['tmp_name'];
    $swfdata = file_get_contents($filename);
    $swfdata_len = strlen($swfdata);
    if ($swfdata_len === 0) {
	header("Content-Type: text/html; charset=utf-8");
        echo "ファイルのサイズを見直して下さい。\n";
        exit(0);
    }
    $upload_max_filesize  = ini_get('upload_max_filesize');
    if ($swfdata_len > $upload_max_filesize_bytes) {
	header("Content-Type: text/html; charset=utf-8");
        echo "$upload_max_filesize Bytes 以内のファイルしか受け付けません。\n";
        exit(0);
    }
    $tmp_name = sha1($swfdata, false);
    $id = substr($tmp_name, 0, 16); // XXX
    $tmp_filename = "$tmp_prefix$id.swf";
    if ((! is_readable($tmp_filename)) &&
        (! file_put_contents($tmp_filename, $swfdata))) {
	header("Content-Type: text/html; charset=utf-8");
        echo "ごめんなさい。内部エラーです。";
        fprintf(STDERR, "index.php: file_put_contents failed. zero size?\n");
        unlink($tmp_filename);
        exit(0);
    }
    header("Location: ./index.php?id=$id");
    exit(0);
}

?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
</head>
<body bgcolor="#f0fff0">
<form enctype="multipart/form-data" action="" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="<?php echo $upload_max_filesize_bytes ?>" />
    SWF ファイルをアップロード: <input name="swffile" type="file" />
    <input type="submit" value="ファイルを送信" />
</form>

<?php
    
if (empty($_REQUEST['id']))  {
     echo "ファイルを指定してください。($upload_max_filesize Bytes 以内に限定してます)";
     exit(0);
}
$id = hex_from_string($_REQUEST['id']);
$tmp_filename = "$tmp_prefix$id.swf";
$swfdata = file_get_contents($tmp_filename);

echo "<a href=\"./swfimagelist.php?id=$id&noshape=y\" target=\"_blank\"> ビットマップ画像一覧 </a> - ";
echo "<a href=\"./swfimagelist.php?id=$id\" target=\"_blank\"> ベクター画像含む(重たいので注意) </a> <br />\n";

$swf = new SWFEditor();
$swf->input($swfdata);

$header_info = $swf->getHeaderInfo();

if ($header_info['compress']) {
    $compress = 'true';
} else {
    $compress = 'false';

}
echo "<table border=1>\n";
echo "<tr> <th> compress </th> </tr>\n";
echo "<tr> <td> $compress </td> </tr>\n";
echo "</table>\n";

echo "<table border=1>\n";
echo "<th> tag </th> <th> length </th> <th> detail </th>\n";
foreach ($swf->getTagList() as $tag_seqno => $tagblock) {
    $tag= $tagblock['tag'];
    if (empty($tagblock['tagName'])) {
        $name = "Unknown";
    } else {
        $name = $tagblock['tagName'];
    }
    $length = $tagblock['length'];
    $detail = @$tagblock['detail'];
    echo "<tr>\n";
    echo "<td> $name($tag) </td> ";
    echo "<td> $length </td>\n";
    if ($detail) {
        $detail_info = $swf->getTagDetail($tag_seqno);
        $detail_str = '';
        if (is_array($detail_info)) {
            foreach ($detail_info as $key => $value) {
                if (is_array($value)) {
                    $value = implode(',', $value);
	            }
	            $detail_str .= "$key($value) ";
            }
        } else {
            $detail_str .= var_export($detail_info, true);
        }

        if (@$header_info['version'] < 6) { // for flash lite
            $detail_str = mb_convert_encoding($detail_str, "UTF-8", "SJIS-win");
        }
        $detail_str = htmlspecialchars($detail_str);
        switch ($tag) {
          case 6:  // 'DefineBitsJPEG'
          case 21: // 'DefineBitsJPEG2'
          case 35: // 'DefineBitsJPEG3'
          case 20: // 'DefineBitsLossless'
          case 36: // 'DefineBitsLossless2'
            $image_id = $detail_info['image_id'];
            $detail_str .= " <a href=\"swfimagereplace.php?id=$id&image_id=$image_id\"> replace </a>";
            break;
        }
        echo "<td> $detail_str </td>";
    } else {
        echo "<td> - </td>";
    }
    echo "</tr>\n";
}
echo "</table>\n";

?>
</body>
</html>
