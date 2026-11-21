--TEST--
replaceActionString() : ActionPush longer parameter
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$params = array('dummy' => 'dummydummy', 'inputvar' => 'replacedvar');
$obj->replaceActionStrings($params);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
75187310c04340714dd97eda2efcd054
===DONE===
