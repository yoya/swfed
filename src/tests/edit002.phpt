--TEST--
replaceEditString() test;
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
var_export($obj->getEditString('textvar'));
var_export($obj->getEditString('text'));
var_export($obj->getEditString(''));
echo PHP_EOL;
?>
===DONE===
--EXPECTF--
'ABCDEFG'falsefalse
===DONE===
