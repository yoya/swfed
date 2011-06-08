--TEST--
setActionVariables() : create doaction
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar-noaction.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$params = array('testvar' => 'dummydummy');
$obj->setActionVariables($params);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
e9e1131d19532069b8f1b86a915a05c6
===DONE===
