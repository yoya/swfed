--TEST--
Declared return types of getTagData() / getSoundData()
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip the PHP 7 arginfo has no types"); ?>
--FILE--
<?php

foreach (array('getTagData', 'getSoundData', 'getEditString', 'getTagDetail') as $name) {
    echo $name, ": ", (new ReflectionMethod('SWFEditor', $name))->getReturnType(), "\n";
}
?>
===DONE===
--EXPECT--
getTagData: string|false
getSoundData: string|false
getEditString: string|false
getTagDetail: array|false
===DONE===
