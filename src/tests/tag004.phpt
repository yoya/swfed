--TEST--
getTagContentsByCID() on an edited tag returns only the contents
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
var_dump($obj->replaceJpegData(1, file_get_contents('tests/5.jpg')));
echo bin2hex(substr($obj->getTagContentsByCID(1), 0, 2)), "\n";

$obj2 = new SWFEditor();
$obj2->input($obj->output());
var_dump(strlen($obj2->getJpegData(1)) > 0);
$d = $obj2->getTagDetail(3);
var_dump($d['image_id']);
?>
===DONE===
--EXPECT--
bool(true)
ffd8
bool(true)
int(1)
===DONE===
