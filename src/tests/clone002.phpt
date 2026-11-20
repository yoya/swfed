--TEST--
clone SWFEditor makes an independent copy
--FILE--
<?php

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
$o->setCompressLevel(1);
$before = $o->output();

$c = clone $o;
var_dump($c->output() === $before);
$c->setHeaderInfo(array('version' => 3));
$c->removeTag(3);
$h = $o->getHeaderInfo();
echo $h['version'], " ", count($o->getTagList()), "\n";
$h = $c->getHeaderInfo();
echo $h['version'], " ", count($c->getTagList()), "\n";
var_dump($o->output() === $before);

unset($o);
var_dump(strlen($c->output()) > 0);

// clone of a fresh object
$e = clone (new SWFEditor());
var_dump($e->input($before));
?>
===DONE===
--EXPECT--
bool(true)
4 85
3 84
bool(true)
bool(true)
bool(true)
===DONE===
