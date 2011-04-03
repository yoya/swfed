--TEST--
getBitmapSize Lossless
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';

$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
$obj->input($swfdata);
$image_id = 1;
foreach ($obj->getBitmapSize($image_id) as $key => $value) {
    echo "$key=$value\n";
}
?>
===DONE===
--EXPECTF--
width=320
height=240
===DONE===
