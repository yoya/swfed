--TEST--
clone SWFEditor is rejected instead of sharing the SWF data
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
try {
    $copy = clone $obj;
    echo "cloned\n";
} catch (Error $e) {
    echo get_class($e), ": ", $e->getMessage(), "\n";
}
$info = $obj->getHeaderInfo();
echo $info['version'], "\n";
?>
===DONE===
--EXPECT--
Error: SWFEditor does not support cloning; serialize via output()/input() instead
4
===DONE===
