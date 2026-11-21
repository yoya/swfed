--TEST--
replaceMovieClip instancename search from nested symbol
--FILE--
<?php

$swf_filename = 'tests/negimiku2_mcnest.swf';
$swf_filename2 = 'tests/saitama3.swf';

$swfdata = file_get_contents($swf_filename);
$swfdata2 = file_get_contents($swf_filename2);
$obj = new SWFEditor();

$obj->input($swfdata);

$obj->replaceMovieClip('miku/negi', $swfdata2);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
fa9c60cf7b8ebbdf5d2be6252b763fdc
===DONE===
