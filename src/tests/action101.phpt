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
693e53942eac06de8a350822626fe04c
===DONE===
