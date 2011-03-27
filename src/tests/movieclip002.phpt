--TEST--
replaceGIFData and Purge
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';
$swf_filename2 = 'tests/saitama3.swf';

$swfdata = file_get_contents($swf_filename);
$swfdata2 = file_get_contents($swf_filename2);
$obj = new SWFEditor();

$obj->input($swfdata);

$obj->replaceMovieClip('miku', $swfdata2);

$obj->purgeUselessContents();

echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
e97cf06492bf9bf7856b83d4bf0aa2a8
===DONE===
