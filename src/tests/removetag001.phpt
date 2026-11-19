--TEST--
removeTag() argument handling
--FILE--
<?php

function codes($obj) {
    return implode(',', array_map(function ($t) { return $t['code']; },
                                  $obj->getTagList()));
}

$swfdata = file_get_contents('tests/negimiku2_mcnest.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
echo codes($obj), "\n";

try {
    $obj->removeTag();
} catch (ArgumentCountError $e) {
    echo get_class($e), "\n";
}
try {
    $obj->removeTag(5, -1, 'x');
} catch (ArgumentCountError $e) {
    echo get_class($e), "\n";
}
echo codes($obj), "\n";

$obj->input($swfdata);
var_dump($obj->removeTag(10, null));
echo codes($obj), "\n";

$obj->input($swfdata);
var_dump($obj->removeTag(10));
echo codes($obj), "\n";

$r = new ReflectionMethod('SWFEditor', 'removeTag');
var_dump($r->getNumberOfRequiredParameters());
?>
===DONE===
--EXPECT--
9,8,6,2,26,35,2,35,2,2,39,35,2,39,26,1,0
ArgumentCountError
ArgumentCountError
9,8,6,2,26,35,2,35,2,2,39,35,2,39,26,1,0
bool(true)
9,8,6,2,26,35,2,35,2,2,35,2,39,26,1,0
bool(true)
9,8,6,2,26,35,2,35,2,2,35,2,39,26,1,0
int(1)
===DONE===
