--TEST--
Reflection of the whole class (php --rc SWFEditor) works
--FILE--
<?php

$s = (string) new ReflectionClass('SWFEditor');
var_dump(strpos($s, 'swfInfo') !== false);
foreach (array('swfInfo', 'purgeUselessContents') as $name) {
    $t = (new ReflectionMethod('SWFEditor', $name))->getReturnType();
    // PHP 7 (legacy arginfo) has no return types
    echo $name, ": ", ($t === null || (string) $t === 'bool') ? 'ok' : (string) $t, "\n";
}
?>
===DONE===
--EXPECT--
bool(true)
swfInfo: ok
purgeUselessContents: ok
===DONE===
