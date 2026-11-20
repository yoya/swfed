--TEST--
replaceBitmapData() with PNG / GIF data fails without libpng / giflib
--SKIPIF--
<?php
ob_start();
phpinfo(INFO_MODULES);
$info = ob_get_clean();
if ((strpos($info, 'PNG support => no') === false) &&
    (strpos($info, 'GIF support => no') === false)) {
    die('skip swfed built with libpng and giflib');
}
?>
--FILE--
<?php
ob_start();
phpinfo(INFO_MODULES);
$info = ob_get_clean();
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
if (strpos($info, 'PNG support => no') !== false) {
    var_dump($o->replaceBitmapData(1, file_get_contents('tests/negimiku001.png')) === false);
}
if (strpos($info, 'GIF support => no') !== false) {
    var_dump($o->replaceBitmapData(1, file_get_contents('tests/inasahama.gif')) === false);
}
?>
===DONE===
--EXPECTF--
%Abool(true)
===DONE===
