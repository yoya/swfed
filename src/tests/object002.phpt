--TEST--
foreach by reference and serialize() don't break SWFEditor
--FILE--
<?php

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
foreach ($o as $k => &$v) {
    echo "property: $k\n";
}
unset($v);
var_dump(count($o->getTagList()));

try {
    serialize($o);
    echo "serialized\n";
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
// PHP 8.1+ throws, older versions warn and return false
try {
    $u = @unserialize('O:9:"SWFEditor":0:{}');
    echo ($u === false) ? "unserialize refused\n" : "unserialized\n";
} catch (Exception $e) {
    echo "unserialize refused\n";
}
?>
===DONE===
--EXPECT--
int(85)
Serialization of 'SWFEditor' is not allowed
unserialize refused
===DONE===
