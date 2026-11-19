--TEST--
input() failure keeps the previously loaded SWF
--FILE--
<?php

$obj = new SWFEditor();
var_dump($obj->input(file_get_contents('tests/saitama.swf')));
$before = $obj->output();
$tags = count($obj->getTagList());
$header = $obj->getHeaderInfo();

// CWS header with broken zlib data
var_dump($obj->input("CWS\x0A" . pack('V', 100) . "not zlib data......"));
var_dump(count($obj->getTagList()) === $tags);
var_dump($obj->getHeaderInfo() === $header);
var_dump($obj->output() === $before);
?>
===DONE===
--EXPECTF--
bool(true)
%Abool(false)
bool(true)
bool(true)
bool(true)
===DONE===
