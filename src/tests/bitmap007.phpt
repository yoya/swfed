--TEST--
A failed replacePNGData() / replaceGIFData() leaves the bitmap tag as it was
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--FILE--
<?php

$swf = file_get_contents('tests/colorformat.swf');
$broken_png = "\x89PNG\r\n\x1a\nbroken";
$broken_gif = "GIF89a broken";

function tag_info($o, $i) {
    $l = $o->getTagList();
    return $l[$i]['tagName'] . " " . $l[$i]['length'];
}

// (a) failing on a DefineBitsJPEG2 must not turn it into a Lossless2 tag
foreach (array('replacePNGData' => $broken_png, 'replaceGIFData' => $broken_gif) as $method => $data) {
    $o = new SWFEditor();
    $o->input($swf);
    $before = tag_info($o, 4);
    $jpeg = $o->getJpegData(4);
    var_dump(@$o->$method(4, $data));
    echo $method, ": ", (tag_info($o, 4) === $before) ? "tag kept" : tag_info($o, 4), "\n";
    $o2 = new SWFEditor();
    $o2->input($o->output());
    var_dump($o2->getJpegData(4) === $jpeg);
}

// (b) failing after a successful replacement must not drop the tag
$o = new SWFEditor();
$o->input($swf);
$count = count($o->getTagList());
var_dump($o->replacePNGData(2, file_get_contents('tests/negimiku001.png')));
$size = $o->getBitmapSize(2);
var_dump(@$o->replacePNGData(2, $broken_png));
$out = $o->output();
var_dump(is_string($out));
$o2 = new SWFEditor();
$o2->input($out);
var_dump(count($o2->getTagList()) === $count);
var_dump($o2->getBitmapSize(2) == $size);
?>
===DONE===
--EXPECTF--
%Abool(false)
replacePNGData: tag kept
bool(true)
%Abool(false)
replaceGIFData: tag kept
bool(true)
bool(true)
%Abool(false)
bool(true)
bool(true)
bool(true)
===DONE===
