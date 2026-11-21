--TEST--
replaceBitmapData(): the warning for a character ID out of range names the argument (#159)
--FILE--
<?php
$o = new SWFEditor();
$o->input(file_get_contents('tests/colorformat.swf'));
var_dump($o->replaceBitmapData(70000, file_get_contents('tests/5.jpg')));
var_dump($o->replaceBitmapData(-1, file_get_contents('tests/5.jpg')));
var_dump($o->replaceBitmapData('65536', file_get_contents('tests/5.jpg')));
?>
===DONE===
--EXPECTF--
Warning: SWFEditor::replaceBitmapData(): bitmap_id(70000) must be between 0 and 65535 in %s on line %d
bool(false)

Warning: SWFEditor::replaceBitmapData(): bitmap_id(-1) must be between 0 and 65535 in %s on line %d
bool(false)

Warning: SWFEditor::replaceBitmapData(): bitmap_id(65536) must be between 0 and 65535 in %s on line %d
bool(false)
===DONE===
