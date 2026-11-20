--TEST--
setHeaderInfo(): compress / lzma together with version / frame size, validated before anything changes
--SKIPIF--
<?php
ob_start();
phpinfo(INFO_MODULES);
if (strpos(ob_get_clean(), 'LZMA (ZWS) support => yes') === false) {
    die('skip swfed built without liblzma');
}
?>
--FILE--
<?php
// what a reader of the output sees: magic, version, frame size
function show($o) {
    $out = $o->output();
    $n = new SWFEditor();
    $n->input($out);
    $h = $n->getHeaderInfo();
    echo substr($out, 0, 3), " v", $h['version'], " ", $h['x_min'], ",", $h['y_min'],
         " ", $h['x_max'], ",", $h['y_max'], " lzma=", var_export($h['lzma'], true), "\n";
}

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
show($o);

// the switch of the compression and version / frame size in one call
var_dump($o->setHeaderInfo(array('compress' => true, 'version' => 9, 'x_max' => 100.5)));
show($o); // CWS
var_dump($o->setHeaderInfo(array('lzma' => true, 'version' => 10, 'y_max' => 90)));
show($o); // ZWS
var_dump($o->setHeaderInfo(array('compress' => false, 'lzma' => true, 'version' => 11, 'x_min' => 5)));
show($o); // FWS (compress => false wins)
var_dump($o->setHeaderInfo(array('compress' => true, 'lzma' => true, 'version' => 12)));
show($o); // ZWS
var_dump($o->setHeaderInfo(array('compress' => true, 'version' => 13)));
show($o); // stays ZWS
var_dump($o->setHeaderInfo(array('compress' => true, 'lzma' => false, 'y_min' => 7)));
show($o); // CWS
var_dump($o->setHeaderInfo(array('compress' => false)));
show($o); // FWS
var_dump($o->setHeaderInfo(array('lzma' => false)));
show($o); // FWS stays

// an invalid value: false and nothing changes, whatever the other keys ask for
$o->setHeaderInfo(array('lzma' => true, 'version' => 8, 'x_min' => 1, 'y_min' => 2, 'x_max' => 3, 'y_max' => 4));
show($o); // ZWS v8 1,2 3,4
var_dump($o->setHeaderInfo(array('compress' => false, 'version' => 300)));
show($o);
var_dump($o->setHeaderInfo(array('lzma' => false, 'x_max' => 53687092, 'version' => 20)));
show($o);
$o->setHeaderInfo(array('compress' => false));
var_dump($o->setHeaderInfo(array('lzma' => true, 'version' => 300)));
show($o);
var_dump($o->setHeaderInfo(array('compress' => true, 'lzma' => true, 'y_max' => -53687092, 'version' => 20)));
show($o);
?>
===DONE===
--EXPECTF--
FWS v4 0,0 240,240 lzma=false
bool(true)
CWS v9 0,0 100.5,240 lzma=false
bool(true)
ZWS v10 0,0 100.5,90 lzma=true
bool(true)
FWS v11 5,0 100.5,90 lzma=false
bool(true)
ZWS v12 5,0 100.5,90 lzma=true
bool(true)
ZWS v13 5,0 100.5,90 lzma=true
bool(true)
CWS v13 5,7 100.5,90 lzma=false
bool(true)
FWS v13 5,7 100.5,90 lzma=false
bool(true)
FWS v13 5,7 100.5,90 lzma=false
ZWS v8 1,2 3,4 lzma=true

Warning: SWFEditor::setHeaderInfo(): version(300) must be between 0 and 255 in %s on line %d
bool(false)
ZWS v8 1,2 3,4 lzma=true

Warning: SWFEditor::setHeaderInfo(): x_max(53687092) must be between -53687091.20 and 53687091.15 in %s on line %d
bool(false)
ZWS v8 1,2 3,4 lzma=true

Warning: SWFEditor::setHeaderInfo(): version(300) must be between 0 and 255 in %s on line %d
bool(false)
FWS v8 1,2 3,4 lzma=false

Warning: SWFEditor::setHeaderInfo(): y_max(-53687092) must be between -53687091.20 and 53687091.15 in %s on line %d
bool(false)
FWS v8 1,2 3,4 lzma=false
===DONE===
