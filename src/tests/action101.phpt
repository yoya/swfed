--TEST--
replaceActionString() : ActionPush shorten parameter
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$params = array('dummydummy' => 'dummy', 'inputvar' => 'rep');
$obj->replaceActionStrings($params);
echo md5($obj->output())."\n";
?>
===DONE===
--EXPECTF--
3d27f4ce7242bdced1ad2a3299673ab8
===DONE===
