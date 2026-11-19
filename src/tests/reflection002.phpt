--TEST--
Reflection parameter types of optional arguments
--SKIPIF--
<?php if (PHP_VERSION_ID < 80000) die("skip the PHP 7 arginfo has no types"); ?>
--FILE--
<?php

foreach (array('removeTag' => 1, 'replaceJpegData' => 2, 'replacePNGData' => 2,
               'replaceBitmapData' => 3, 'replaceMovieClip' => 2) as $name => $i) {
    $p = (new ReflectionMethod('SWFEditor', $name))->getParameters()[$i];
    echo $name, ": ", $p->getType(), " $", $p->getName(),
         " = ", var_export($p->getDefaultValue(), true), "\n";
}
?>
===DONE===
--EXPECT--
removeTag: ?int $index_sprint = NULL
replaceJpegData: ?string $alpha_data = NULL
replacePNGData: ?array $option = NULL
replaceBitmapData: mixed $arg4 = NULL
replaceMovieClip: ?bool $unused_cid_purge = NULL
===DONE===
