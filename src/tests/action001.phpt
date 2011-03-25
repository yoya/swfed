--TEST--
setActionVariables() : merge to doaction
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$params = array('dummy' => 'dummydummy');
$obj->setActionVariables($params);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
120f1086954ded7c3110609fa12cf90d
===DONE===
