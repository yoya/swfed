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
4d58d118b83343655a61dc5e023c26e3
===DONE===
