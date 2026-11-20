--TEST--
disasmActionData() disassembles actions
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/textvar.swf'));
$obj->disasmActionData($obj->getActionData(1));
exit(0);
?>
--EXPECTF--
%wPush Data%A
%wSet Variable
%wEnd
