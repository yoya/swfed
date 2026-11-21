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
9cb69a1ef6c7cbd31cfc97b3a313a23b
===DONE===
