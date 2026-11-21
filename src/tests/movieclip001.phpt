--TEST--
replaceMovieClip
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';
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
407de1fa0feafe4e29c1d8dfef953efc
===DONE===
