--TEST--
setCompressLevel() rejects values outside zlib range
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
$obj->setHeaderInfo(array('compress' => true));
foreach (array(10, -5, 100) as $level) {
    var_dump($obj->setCompressLevel($level));
}
var_dump(is_string($obj->output()));
foreach (array(-1, 0, 9) as $level) {
    var_dump($obj->setCompressLevel($level));
    var_dump(substr($obj->output(), 0, 3));
}
?>
===DONE===
--EXPECTF--
Warning: SWFEditor::setCompressLevel(): setCompressLevel: compress_level must be -1..9 (got 10) in %s on line %d
bool(false)

Warning: SWFEditor::setCompressLevel(): setCompressLevel: compress_level must be -1..9 (got -5) in %s on line %d
bool(false)

Warning: SWFEditor::setCompressLevel(): setCompressLevel: compress_level must be -1..9 (got 100) in %s on line %d
bool(false)
bool(true)
bool(true)
string(3) "CWS"
bool(true)
string(3) "CWS"
bool(true)
string(3) "CWS"
===DONE===
