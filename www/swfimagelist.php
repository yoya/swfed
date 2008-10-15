<?php

require_once('define.php');

if (@$argc > 1) {
    $id = $argv[1];
} else {
    $id = $_REQUEST['id'];
}

$filename = "$tmp_prefix$id.swf";
$swfdata = file_get_contents($filename);

$swf = new SWFEditor();
$swf->input($swfdata);

echo "<table border=1>\n";
echo "<th> tag </th> <th> image_id </th> <th> image </th>\n";
foreach ($swf->getTagList() as $tag_seqno => $tagblock) {
    $tag= $tagblock['tag'];
    $ext = '';
    if (($tag == 6) || ($tag == 21) || ($tag == 35)) {
        $ext = '.jpg';
    }
    if (($tag == 20) || ($tag == 36)) {
        $ext = '.png';
    }
    if (empty($ext)) {
        continue;
    }
    $name = $tagblock['tagName'];
    $detail = $tagblock['detail'];
    $detail_info = $swf->getTagDetail($tag_seqno);
    $image_id = $detail_info['image_id'];
    $image_filename = "$tmp_prefix$id-$image_id$ext";
    if ($ext == '.jpg') {
        $image_data = $swf->getJpegData(intval($image_id));
    } else {
        $image_data = $swf->getPNGData(intval($image_id));
    }
    file_put_contents($image_filename, $image_data);
    echo "<tr>\n";
    echo "<td> $name($tag) </td> ";
    echo "<td> $image_id (ext=$ext)</td>\n";
    echo "<td> <img src=\"./swfimage.php?id=$id&image_id=$image_id&ext=$ext\"> </td>";
    echo "</tr>\n";
}
echo "</table>\n";

?>
</body>
</html>
