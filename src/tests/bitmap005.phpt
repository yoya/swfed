--TEST--
replaceBitmapData Lossless with alpha0
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--FILE--
<?php

$swf_filename = 'tests/colorformat_trans.swf';
$png_filename = 'tests/negimiku001.png';

$swfdata = file_get_contents($swf_filename);

$obj = new SWFEditor();
$obj->input($swfdata);

$image_cond = array('red' => 0, 'green' => 0, 'blue' => 0);
$pngdata = file_get_contents($png_filename);
$obj->replaceBitmapData($image_cond, $pngdata);

echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
38a7ffcab67450fd52b4f7a9b40cd3c1
===DONE===
