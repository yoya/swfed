--TEST--
setHeaderInfo() / tag index / CID range checks, getHeaderInfo() round trip
--FILE--
<?php

function reload($o) {
    $n = new SWFEditor();
    $n->input($o->output());
    return $n->getHeaderInfo();
}

$swf = file_get_contents('tests/saitama.swf');
$o = new SWFEditor();
$o->input($swf);

var_dump($o->setHeaderInfo(array('x_max' => 53687092)));
var_dump($o->setHeaderInfo(array('version' => 264)));
var_dump($o->setHeaderInfo(array('version' => 8, 'x_max' => -53687092)));
$h = reload($o);
echo $h['version'], " ", $h['x_max'], "\n";
var_dump($o->setHeaderInfo(array('x_max' => 53687091)));
$h = reload($o);
echo $h['x_max'], "\n";

// fractional frame size survives get -> set
var_dump($o->setHeaderInfo(array('x_min' => 0, 'y_min' => 0, 'x_max' => 275.5, 'y_max' => 200.5)));
$h = reload($o);
var_dump($h['x_max'], $h['y_max']);
$h['version'] = 8;
$o->setHeaderInfo($h);
$h = reload($o);
var_dump($h['version'], $h['x_max'], $h['y_max']);

// tag index / CID are not wrapped around 2^32
$o->input($swf);
$n = count($o->getTagList());
var_dump($o->getTagData(4294967296));
var_dump($o->getJpegData(4294967297));
var_dump($o->removeTag(4294967296 + 2));
var_dump(count($o->getTagList()) === $n);
var_dump(strlen($o->getJpegData(1)) > 0);
?>
===DONE===
--EXPECTF--
Warning: SWFEditor::setHeaderInfo(): x_max(53687092) must be between -53687091.20 and 53687091.15 in %s on line %d
bool(false)

Warning: SWFEditor::setHeaderInfo(): version(264) must be between 0 and 255 in %s on line %d
bool(false)

Warning: SWFEditor::setHeaderInfo(): x_max(-53687092) must be between -53687091.20 and 53687091.15 in %s on line %d
bool(false)
4 240
bool(true)
53687091
bool(true)
float(275.5)
float(200.5)
int(8)
float(275.5)
float(200.5)

Warning: SWFEditor::getTagData(): tag_seqno(4294967296) must be between 0 and 2147483647 in %s on line %d
bool(false)

Warning: SWFEditor::getJpegData(): image_id(4294967297) must be between 0 and 65535 in %s on line %d
bool(false)

Warning: SWFEditor::removeTag(): tag_seqno(4294967298) must be between 0 and 2147483647 in %s on line %d
bool(false)
bool(true)
bool(true)
===DONE===
