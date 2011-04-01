--TEST--
replaceTagData
--FILE--
<?php

$swf_filename = 'tests/negimiku.swf';
$tag_filename = 'tests/saitama-3.bits';

$swfdata = file_get_contents($swf_filename);
$tagdata = file_get_contents($tag_filename);

$obj = new SWFEditor();
$obj->input($swfdata);
$tag_seqno = 2;
$obj->replaceTagData($tag_seqno, $tagdata);
echo md5($obj->output())."\n";
# echo $obj->output();
?>
===DONE===
--EXPECTF--
efbdb13e9f04f73c80951bb049423d2e
===DONE===
