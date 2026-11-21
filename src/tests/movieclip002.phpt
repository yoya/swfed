--TEST--
replaceMovieClip and Purge
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
f59ee063b8e2ae4a6e96ef7e33873c4a
===DONE===
