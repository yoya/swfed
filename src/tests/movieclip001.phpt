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
9ecdd9f333c36b03f76dacb272efd0b7
===DONE===
