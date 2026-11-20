--TEST--
setActionVariables() rejects values longer than a Push action can hold
--FILE--
<?php

$swfdata = file_get_contents('tests/textvar.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
$before = $obj->output();
// the return value is covered by action203.phpt; here the SWF must stay intact
$obj->setActionVariables(array('key' => str_repeat('x', 70000)));
var_dump($obj->output() === $before);

var_dump($obj->setActionVariables(array('key' => 'short')));
var_dump(strpos($obj->output(), "\0key\0") !== false &&
         strpos($obj->output(), "\0short\0") !== false);
?>
===DONE===
--EXPECTF--
%Abool(true)
bool(true)
bool(true)
===DONE===
