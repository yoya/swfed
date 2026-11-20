--TEST--
getTagList() length is up to date after editing / rebuild()
--FILE--
<?php

// body length of the tag as actually written
function body_length($o, $i) {
    $d = $o->getTagData($i);
    return strlen($d) - (((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2);
}
function check($o, $i) {
    $l = $o->getTagList();
    echo $l[$i]['tagName'], " ", ($l[$i]['length'] === body_length($o, $i)) ? "ok" : "length=" . $l[$i]['length'] . " actual=" . body_length($o, $i), "\n";
}

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
var_dump($o->replaceJpegData(1, file_get_contents('tests/5.jpg')));
check($o, 3);

$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->rebuild());
check($o, 5);

$o->input(file_get_contents('tests/textvar.swf'));
$o->setActionVariables(array('foo' => 'bar'));
check($o, 1);
?>
===DONE===
--EXPECT--
bool(true)
DefineBitsJPEG2 ok
bool(true)
DefineBitsJPEG3 ok
DoAction ok
===DONE===
