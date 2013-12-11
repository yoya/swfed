--TEST--
setActionVariables() : merge to doaction (no 0 terminate record)
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar-no0.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$params = array('dummy' => 'dummydummy');
$obj->setActionVariables($params);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
e57a990a6b0205dae28fb879c3b17acb
===DONE===
