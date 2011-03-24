--TEST--
replaceGIFData test;
--FILE--
<?php

$swf_filename = 'tests/negi.swf';
$swf_filename2 = 'tests/saitama3.swf';

$swfdata = file_get_contents($swf_filename);
$swfdata2 = file_get_contents($swf_filename2);
$obj = new SWFEditor();

$obj->input($swfdata);

$obj->replaceMovieClip('miku', $swfdata2);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
967f0de27ae9f02cf658000a0b802fc6
===DONE===
