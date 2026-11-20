--TEST--
getLastError() is empty after a method call that does not fail in the library, however it returns
--INI--
swfed.error_output=none
--FILE--
<?php
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
$saitama_cws = 'CWS' . substr(file_get_contents('tests/saitama.swf'), 3, 5)
             . gzcompress(substr(file_get_contents('tests/saitama.swf'), 8));

// every call below starts with an error left by a failed call
function after_error($o, $name, $f) {
    $o->getShapeData(1); // DefineBitsJPEG: not a shape
    if (SWFEditor::getLastError() === null) {
        echo "$name: no error to start with\n";
        return;
    }
    set_error_handler(function ($no, $str) { return true; }); // PHP 7 warnings
    ob_start(); // disasmActionData() prints
    try {
        $f($o);
    } catch (Throwable $e) {
    }
    ob_end_clean();
    restore_error_handler();
    echo "$name: ", var_export(SWFEditor::getLastError(), true), "\n";
}

echo "* methods that do not go through the movie\n";
after_error($o, 'isShapeTagData', function ($o) { return $o->isShapeTagData("\x00\x00"); });
after_error($o, 'isBitmapTagData', function ($o) { return $o->isBitmapTagData("\x00\x00"); });
after_error($o, 'disasmActionData', function ($o) { return $o->disasmActionData("\x00"); });
after_error($o, 'setCompressLevel', function ($o) { return $o->setCompressLevel(9); });
after_error($o, 'getHeaderInfo', function ($o) { return $o->getHeaderInfo(); });

echo "* returns before the library: wrong arguments\n";
after_error($o, 'getShapeData(string)', function ($o) { return $o->getShapeData('abc'); });
after_error($o, 'getShapeData()', function ($o) { return $o->getShapeData(); });
after_error($o, 'input()', function ($o) { return $o->input(); });
after_error($o, 'setHeaderInfo(string)', function ($o) { return $o->setHeaderInfo('x'); });

echo "* returns before the library: range checks\n";
after_error($o, 'getShapeData(70000)', function ($o) { return $o->getShapeData(70000); });
after_error($o, 'getTagData(-1)', function ($o) { return $o->getTagData(-1); });
after_error($o, 'setHeaderInfo(version)', function ($o) { return $o->setHeaderInfo(array('version' => 1000)); });
after_error($o, 'setCompressLevel(10)', function ($o) { return $o->setCompressLevel(10); });

echo "* returns before the library: swfed.max_uncompressed_size\n";
ini_set('swfed.max_uncompressed_size', '1K');
after_error($o, 'input(CWS)', function ($o) use ($saitama_cws) { return $o->input($saitama_cws); });
after_error($o, 'replaceMovieClip(CWS)', function ($o) use ($saitama_cws) { return $o->replaceMovieClip('x', $saitama_cws); });
ini_set('swfed.max_uncompressed_size', '0');

echo "* the errors of a call stay until the next call\n";
$o->getShapeData(1);
echo SWFEditor::getLastError() === SWFEditor::getLastError() ? "same\n" : "different\n";
echo strtok(SWFEditor::getLastError(), "\n"), "\n";
$o->getShapeData(1);
echo substr_count(SWFEditor::getLastError(), "\n"), "\n"; // not piled up
echo "alive\n";
?>
===DONE===
--EXPECT--
* methods that do not go through the movie
isShapeTagData: NULL
isBitmapTagData: NULL
disasmActionData: NULL
setCompressLevel: NULL
getHeaderInfo: NULL
* returns before the library: wrong arguments
getShapeData(string): NULL
getShapeData(): NULL
input(): NULL
setHeaderInfo(string): NULL
* returns before the library: range checks
getShapeData(70000): NULL
getTagData(-1): NULL
setHeaderInfo(version): NULL
setCompressLevel(10): NULL
* returns before the library: swfed.max_uncompressed_size
input(CWS): NULL
replaceMovieClip(CWS): NULL
* the errors of a call stay until the next call
same
swf_object_get_shapedata: not isShapeTag(6)
1
alive
===DONE===
