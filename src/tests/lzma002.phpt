--TEST--
LZMA compressed SWF (ZWS) without liblzma
--SKIPIF--
<?php
ob_start();
phpinfo(INFO_MODULES);
if (strpos(ob_get_clean(), 'LZMA (ZWS) support => no') === false) {
    die('skip swfed built with liblzma');
}
?>
--FILE--
<?php
$o = new SWFEditor();
var_dump($o->input(file_get_contents('tests/saitama_lzma.swf')));
var_dump($o->input(file_get_contents('tests/saitama.swf')));
var_dump($o->setHeaderInfo(array('lzma' => true)));
echo substr($o->output(), 0, 3), "\n";
// a failed call changes nothing: not the version, not the frame size either
$before = $o->getHeaderInfo();
var_dump($o->setHeaderInfo(array('lzma' => true, 'version' => 9, 'x_max' => 100)));
var_dump($o->getHeaderInfo() === $before);
var_dump($o->setHeaderInfo(array('compress' => true, 'lzma' => true, 'version' => 9)));
var_dump($o->getHeaderInfo() === $before);
// without lzma it works as before
var_dump($o->setHeaderInfo(array('compress' => true, 'version' => 9, 'x_max' => 100)));
var_dump($o->getHeaderInfo() === $before, substr($o->output(), 0, 3));
?>
===DONE===
--EXPECTF--
%Abool(false)
bool(true)

Warning: SWFEditor::setHeaderInfo(): LZMA (ZWS) is not supported (built without liblzma) in %s on line %d
bool(false)
FWS

Warning: SWFEditor::setHeaderInfo(): LZMA (ZWS) is not supported (built without liblzma) in %s on line %d
bool(false)
bool(true)

Warning: SWFEditor::setHeaderInfo(): LZMA (ZWS) is not supported (built without liblzma) in %s on line %d
bool(false)
bool(true)
bool(true)
bool(false)
string(3) "CWS"
===DONE===
