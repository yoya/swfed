--TEST--
getShapeData(), replaceShapeData(), applyShapeMatrixFactor(), applyShapeRectFactor()
--FILE--
<?php
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
$before = $o->output();

echo "* getShapeData / replaceShapeData\n";
$shape = $o->getShapeData(2);
var_dump($shape === $o->getTagData(3));
var_dump($o->replaceShapeData(2, $shape));
var_dump($o->output() === $before);
var_dump($o->getShapeData(1)); // DefineBitsJPEG

echo "* applyShapeMatrixFactor\n";
var_dump($o->applyShapeMatrixFactor(2, 2.0, 2.0, 0.0, 10, 20));
$after = $o->output();
var_dump($after !== $before);
$o2 = new SWFEditor();
var_dump($o2->input($after));
echo json_encode($o2->getTagDetail(3)), "\n";
var_dump($o->applyShapeMatrixFactor(1, 2.0, 2.0, 0.0, 0, 0)); // not a shape

echo "* applyShapeRectFactor\n";
var_dump($o->applyShapeRectFactor(4, 0.5, 0.5, 0, 0));
var_dump($o->output() !== $after);
var_dump($o->applyShapeRectFactor(99, 0.5, 0.5, 0, 0));
?>
===DONE===
--EXPECTF--
* getShapeData / replaceShapeData
bool(true)
bool(true)
bool(true)
%Abool(false)
* applyShapeMatrixFactor
bool(true)
bool(true)
bool(true)
{"shape_id":2,"fill_styles.count":2,"line_styles.count":0,"bitmap_ref":[1]}
%Abool(false)
* applyShapeRectFactor
bool(true)
bool(true)
%Abool(false)
===DONE===
