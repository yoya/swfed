--TEST--
Reflection signatures match optional arguments
--FILE--
<?php

foreach (array('removeTag', 'replaceJpegData', 'replacePNGData',
               'replaceBitmapData', 'replaceMovieClip') as $name) {
    $r = new ReflectionMethod('SWFEditor', $name);
    echo $name, ": required=", $r->getNumberOfRequiredParameters(),
         " total=", $r->getNumberOfParameters(), "\n";
}
$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
var_dump($obj->replacePNGData(1, file_get_contents('tests/negimiku001.png')));
var_dump($obj->replacePNGData(1, file_get_contents('tests/negimiku001.png24'), array('rgb15' => true)));
?>
===DONE===
--EXPECT--
removeTag: required=1 total=2
replaceJpegData: required=2 total=3
replacePNGData: required=2 total=3
replaceBitmapData: required=2 total=4
replaceMovieClip: required=2 total=3
bool(true)
bool(true)
===DONE===
