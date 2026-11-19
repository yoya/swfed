--TEST--
swfInfo() goes through the output buffer, errors go to getLastError() and swfed.error_output
--INI--
swfed.error_output=none
--FILE--
<?php
var_dump(ini_get('swfed.error_output'));
$o = new SWFEditor();
var_dump(SWFEditor::getLastError());
var_dump($o->input(file_get_contents('tests/saitama.swf')));

echo "* swfInfo\n";
ob_start();
$ret = $o->swfInfo();
$info = ob_get_clean();
var_dump($ret);
echo substr($info, 0, strpos($info, "\n")), "\n";
var_dump(substr_count($info, "\n") > 80);

echo "* getLastError\n";
var_dump($o->getShapeData(1)); // DefineBitsJPEG
var_dump(SWFEditor::getLastError());
var_dump($o->getHeaderInfo()['version']); // clears
var_dump(SWFEditor::getLastError());
var_dump($o->input('FWS'));
var_dump(SWFEditor::getLastError());

echo "* warning\n";
ini_set('swfed.error_output', 'warning');
var_dump($o->input('FWS'));

echo "* stderr\n";
ini_set('swfed.error_output', 'stderr');
$cmd = escapeshellarg(PHP_BINARY) . ' -n'
     . ' -d ' . escapeshellarg('extension_dir=' . ini_get('extension_dir'))
     . ' -d extension=swfed'
     . ' -r ' . escapeshellarg('$o = new SWFEditor(); var_dump($o->input("FWS"));');
$p = proc_open($cmd, array(1 => array('pipe', 'w'), 2 => array('pipe', 'w')), $pipes);
echo "stdout: ", stream_get_contents($pipes[1]);
echo "stderr: ", stream_get_contents($pipes[2]);
fclose($pipes[1]);
fclose($pipes[2]);
proc_close($p);
?>
===DONE===
--EXPECTF--
string(0) ""
NULL
bool(true)
* swfInfo
bool(true)
magic=FWS  version=4  file_length=2704
bool(true)
* getLastError
bool(false)
string(96) "swf_object_get_shapedata: not isShapeTag(6)
getShapeData: Can't swf_object_get_shapedata (cid=1)"
int(4)
NULL
bool(false)
string(37) "swf_header_parse: input too short (3)"
* warning

Warning: SWFEditor::input(): swf_header_parse: input too short (3) in %s on line %d
bool(false)
* stderr
stdout: bool(false)
stderr: swf_header_parse: input too short (3)
===DONE===
