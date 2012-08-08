--TEST--
replaceMovieClip with font specify
--FILE--
<?php

$swf_filename = 'tests/font_test.swf';
$swf_filename2 = 'tests/font_test_material.swf';

$swfdata = file_get_contents($swf_filename);
$swfdata2 = file_get_contents($swf_filename2);
$obj = new SWFEditor();

$obj->input($swfdata);

$obj->replaceMovieClip('a', $swfdata2);

$obj->purgeUselessContents();

echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
1502a48cc134cabc41df6130d5d88ae8
===DONE===
