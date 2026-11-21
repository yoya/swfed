--TEST--
replaceTagDataByCID
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';
$tag_filename = 'tests/saitama-3.bits';

$swfdata = file_get_contents($swf_filename);
$tagdata = file_get_contents($tag_filename);

$obj = new SWFEditor();
$obj->input($swfdata);
$image_id = 3;
$obj->replaceTagDataByCID($image_id, $tagdata);
echo md5($obj->output())."\n";
# echo $obj->output();
?>
===DONE===
--EXPECTF--
7a81e5f6cbc3d46ee7f0388a4089ce75
===DONE===
