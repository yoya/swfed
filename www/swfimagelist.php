<html>
<body bgcolor="#f0f0ff">
<?php

require_once('define.php');

$id = $_REQUEST['id'];

$filename = "$tmp_prefix$id.swf";
$swfdata = file_get_contents($filename);

$swf = new SWFEditor();
$swf->input($swfdata);

echo "<table border=1>\n";
echo "<th> tag </th> <th> cid </th> <th> image </th>\n";
$tag_list = $swf->getTagList();

foreach ($tag_list as $tag_seqno => &$tagblock) {
	$tagblock['detail_info'] = $swf->getTagDetail($tag_seqno);
}

// getTagDetail 後に getShape が動かないバグの対処で仕方なく
$swf = new SWFEditor();
$swf->input($swfdata);

foreach ($tag_list as $tag_seqno => $tagblock) {
    $tag= $tagblock['tag'];
    $ext = '';
    if (($tag == 6) || ($tag == 21) || ($tag == 35)) {
        $ext = '.jpg';
    }
    if (($tag == 20) || ($tag == 36)) {
        $ext = '.png';
    }
    if (($tag == 2) || ($tag == 22) || ($tag == 32)) {
        if (empty($_REQUEST['noshape'])) {
            $ext = '.shape';
        }
    }
    if (empty($ext)) {
        continue;
    }

    $name = $tagblock['tagName'];
    $detail = $tagblock['detail'];
    $detail_info = $tagblock['detail_info'];
    if (isset($detail_info['image_id'])) {
	    $cid = $detail_info['image_id'];
    } else {
	    $cid = $detail_info['shape_id'];
    }
    $image_filename = "$tmp_prefix$id-$cid$ext";

    if ($ext == '.jpg') {
        $image_data = $swf->getJpegData(intval($cid));
    } elseif($ext == '.png') {
        $image_data = $swf->getPNGData(intval($cid));
    } else { // shape
        $image_data = $swf->getShapeData(intval($cid));
    }
    if (! is_readable($image_filename)) {
        file_put_contents($image_filename, $image_data);
    }
    echo "<tr>\n";
    echo "<td> $name($tag) </td> ";
    echo "<td> $cid (ext=$ext)</td>\n";

    if (($ext == '.jpg') || ($ext == '.png')) {
	echo "<td> <img src=\"./swfimage.php?id=$id&image_id=$cid&ext=$ext\"> </td>";
    } elseif ($ext == '.shape') { // shape
	echo "<td>
<object width='240' height='240'>
<param name='movie' value='./swfshape.php?id=$id&shape_id=$cid&ext=$ext'>
<param name='wmode' value='transparent'>
<embed src='./swfshape.php?id=$id&shape_id=$cid&ext=$ext' width='240' height='240' type='application/x-shockwave-flash' wmode='transparent'>
</embed>
</object>
</td>";
    }
    echo "</tr>\n";
}
echo "</table>\n";

?>
</body>
</html>
