--TEST--
replaceMovieClip 300 defineshape & placeobject
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';
$swf_filename2 = 'tests/4color-300.swf';

$swfdata = file_get_contents($swf_filename);
$swfdata2 = file_get_contents($swf_filename2);
$obj = new SWFEditor();

$obj->input($swfdata);

$obj->replaceMovieClip('miku', $swfdata2);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
72a21c2e2100a7f14fc9785341636541
===DONE===
