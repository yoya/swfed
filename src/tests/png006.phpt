--TEST--
libpng's own errors and warnings go through getLastError() / swfed.error_output, not straight to stderr
--SKIPIF--
<?php ob_start(); phpinfo(INFO_MODULES); if (strpos(ob_get_clean(), 'PNG support => yes') === false) die('skip swfed built without libpng'); ?>
--INI--
swfed.error_output=none
--FILE--
<?php
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
$png = file_get_contents('tests/negimiku001.png');

// the CRC of the IDAT chunk is wrong: a libpng error
$pos = strpos($png, 'IDAT');
$len = unpack('N', substr($png, $pos - 4, 4))[1];
$bad_crc = $png;
$bad_crc[$pos + 4 + $len] = chr(ord($bad_crc[$pos + 4 + $len]) ^ 0xff);
// a gAMA chunk with the wrong length: a libpng warning (the PNG is still usable)
$ihdr_end = 8 + 8 + 13 + 4;
$bad_gama = substr($png, 0, $ihdr_end)
          . pack('N', 3) . 'gAMA' . "abc" . pack('N', crc32('gAMA' . "abc"))
          . substr($png, $ihdr_end);

echo "* error\n";
var_dump($o->replacePNGData(1, $bad_crc));
$lines = explode("\n", (string) SWFEditor::getLastError());
echo preg_match('/^libpng error: /', $lines[0]) ? "libpng error in getLastError()\n" : "no libpng error: $lines[0]\n";

echo "* warning\n";
var_dump($o->replacePNGData(1, $bad_gama));
$lines = explode("\n", (string) SWFEditor::getLastError());
echo preg_match('/^libpng warning: /', $lines[0]) ? "libpng warning in getLastError()\n" : "no libpng warning: $lines[0]\n";

echo "* swfed.error_output=warning\n";
ini_set('swfed.error_output', 'warning');
$messages = array();
set_error_handler(function ($no, $str) use (&$messages) { $messages[] = $str; return true; });
var_dump($o->replacePNGData(1, $bad_crc));
restore_error_handler();
echo preg_match('/^SWFEditor::replacePNGData\(\): libpng error: /', $messages[0]) ? "E_WARNING\n" : "no E_WARNING: $messages[0]\n";
var_dump(count($messages) > 1);

echo "* still works\n";
var_dump($o->replacePNGData(1, $png));
var_dump(SWFEditor::getLastError());
?>
===DONE===
--EXPECT--
* error
bool(false)
libpng error in getLastError()
* warning
bool(true)
libpng warning in getLastError()
* swfed.error_output=warning
bool(false)
E_WARNING
bool(true)
* still works
bool(true)
NULL
===DONE===
