--TEST--
setActionVariables() leaves the SWF untouched when a value can't be converted
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip PHP 7 raises a recoverable fatal error"); ?>
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/textvar-noaction.swf'));
$n = count($obj->getTagList());
try {
    $obj->setActionVariables(array('a' => new stdClass));
} catch (Error $e) {
    echo get_class($e), ": ", $e->getMessage(), "\n";
}
var_dump(count($obj->getTagList()) === $n);
?>
===DONE===
--EXPECT--
Error: Object of class stdClass could not be converted to string
bool(true)
===DONE===
