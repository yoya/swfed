--TEST--
Wrong argument count / type raises catchable errors instead of fatal ones
--FILE--
<?php

// PHP 8: ArgumentCountError / TypeError, PHP 7: warning and false / NULL
function rejected($f) {
    try {
        $r = @$f();
        return (($r === false) || ($r === null)) ? "rejected" : "accepted";
    } catch (Throwable $e) {
        return "rejected " . get_class($e);
    }
}

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
foreach (array('getEditString', 'replaceEditString', 'getSoundData', 'replaceMLDData',
               'replaceMovieClip', 'replaceBitmapData', 'applyShapeMatrixFactor',
               'applyShapeRectFactor') as $m) {
    $r = rejected(function () use ($o, $m) { return $o->$m(); });
    echo $m, ": ", (PHP_VERSION_ID >= 80000) ? $r : preg_replace('/ .*/', '', $r), "\n";
}
$r = rejected(function () use ($o) { return $o->setHeaderInfo('not-an-array'); });
echo "setHeaderInfo: ", (PHP_VERSION_ID >= 80000) ? $r : preg_replace('/ .*/', '', $r), "\n";
$r = rejected(function () use ($o) { return $o->replaceBitmapData(1, array()); });
echo "replaceBitmapData: ", (PHP_VERSION_ID >= 80000) ? $r : preg_replace('/ .*/', '', $r), "\n";
echo "alive\n";
?>
===DONE===
--EXPECTREGEX--
getEditString: rejected( ArgumentCountError)?
replaceEditString: rejected( ArgumentCountError)?
getSoundData: rejected( ArgumentCountError)?
replaceMLDData: rejected( ArgumentCountError)?
replaceMovieClip: rejected( ArgumentCountError)?
replaceBitmapData: rejected( ArgumentCountError)?
applyShapeMatrixFactor: rejected( ArgumentCountError)?
applyShapeRectFactor: rejected( ArgumentCountError)?
setHeaderInfo: rejected( TypeError)?
replaceBitmapData: rejected( TypeError)?
alive
===DONE===
