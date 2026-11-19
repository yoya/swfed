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
?>
===DONE===
--EXPECTF--
%Abool(false)
bool(true)

Warning: SWFEditor::setHeaderInfo(): LZMA (ZWS) is not supported (built without liblzma) in %s on line %d
bool(false)
FWS
===DONE===
