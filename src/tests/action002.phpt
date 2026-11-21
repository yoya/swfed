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
de146b50d860d0516b9ee8773926e3e3
===DONE===
