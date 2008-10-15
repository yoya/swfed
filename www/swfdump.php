<?php require_once('define.php') ?>

<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
</head>
<body>
<form enctype="multipart/form-data" action="" method="POST">
    <input type="hidden" name="MAX_FILE_SIZE" value="67108864" />
    SWF ファイルをアップロード: <input name="swffile" type="file" />
    <input type="submit" value="ファイルを送信" />
</form>
<?php

if (@$argc > 1) {
    if (! is_array($_FILES)) {
        $_FILES = array();
        $_FILES['swffile'] = array();
    }
    $_FILES['swffile']['tmp_name'] = $argv[1];
}

if (! empty($_FILES['swffile']['tmp_name'])) {
    $filename = $_FILES['swffile']['tmp_name'];
    $swfdata = file_get_contents($filename);
    if ($swfdata > 67108864) {
        echo ' 64M Bytes 以内のファイルしか受け付けません。'."\n";
        exit(0);
    }
    $tmp_name = sha1($swfdata, false);
    $id = substr($tmp_name, 0, 16); // XXX
    $tmp_filename = "$tmp_prefix$id.swf";
    if (! file_put_contents($tmp_filename, $swfdata)) {
        echo 'ERROR: file_put_contents failed.  zero size ?'."\n";
        unlink($tmp_filename);
        exit(0);
    }
    header("Location: ./swfdump.php?id=$id");
    exit(0);
} else {
   if (empty($_REQUEST['id']))  {
        echo "ファイルを指定してください。(64MBytes 以内に限定してます)";
        exit(0);
   }
   $id = $_REQUEST['id'];
   $tmp_filename = "$tmp_prefix$id.swf";
   $swfdata = file_get_contents($tmp_filename);
}

echo "<a href=\"./swfimagelist.php?id=$id\"> 画像一覧 </a> <br />\n";

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
        foreach ($detail_info as $key => $value) {
            $detail_str .= "$key($value) ";
        }

        if (@$header_info['version'] < 6) { // for flash lite
            $detail_str = mb_convert_encoding($detail_str, "UTF-8", "SJIS-win");
        }
        echo "<td> ".htmlspecialchars($detail_str)."</td>";
    } else {
        echo "<td> - </td>";
    }
    echo "</tr>\n";
}
echo "</table>\n";

?>
</body>
</html>
