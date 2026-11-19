--TEST--
removeTag() argument handling
--FILE--
<?php

function codes($obj) {
    return implode(',', array_map(function ($t) { return $t['code']; },
                                  $obj->getTagList()));
}
// PHP 8: ArgumentCountError, PHP 7: warning and false
function rejected($f) {
    try {
        return (@$f() === false) ? "rejected" : "accepted";
    } catch (ArgumentCountError $e) {
        return "rejected";
    }
}

$swfdata = file_get_contents('tests/negimiku2_mcnest.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
echo codes($obj), "\n";

echo rejected(function () use ($obj) { return $obj->removeTag(); }), "\n";
echo rejected(function () use ($obj) { return $obj->removeTag(5, -1, 'x'); }), "\n";
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
rejected
rejected
9,8,6,2,26,35,2,35,2,2,39,35,2,39,26,1,0
bool(true)
9,8,6,2,26,35,2,35,2,2,35,2,39,26,1,0
bool(true)
9,8,6,2,26,35,2,35,2,2,35,2,39,26,1,0
int(1)
===DONE===
