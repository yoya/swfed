--TEST--
replaceEditString() test;
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$obj->replaceEditString('textvar', 'hello world');
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
27f717a0448f3469b9a2ce819309e7ed
===DONE===
