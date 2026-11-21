--TEST--
replaceJpegData() / getJpegData(): the time does not grow with the square of the number of JPEG segments (#147)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

$jpeg = file_get_contents('tests/5.jpg');
$sos = strpos($jpeg, "\xff\xda");
$n = 200000;
$cases = array(
    // restart markers after the SOS header, no EOI: every RSTn is a segment
    'restart markers' => substr($jpeg, 0, $sos) . "\xff\xda\x00\x08\x01\x01\x00\x00\x3f\x00" . str_repeat("\xff\xd0", $n),
    // empty APP1 and DQT segments in turn, before the real ones
    'APP1 and DQT segments' => substr($jpeg, 0, 2) . str_repeat("\xff\xe1\x00\x02\xff\xdb\x00\x02", $n / 2) . substr($jpeg, 2),
);
foreach ($cases as $name => $j) {
    $o = new SWFEditor();
    $o->input(file_get_contents('tests/colorformat.swf'));
    $t = microtime(true);
    $r = $o->replaceJpegData(4, $j);
    $out = $o->getJpegData(4);
    $elapsed = microtime(true) - $t;
    // every segment is still there (the DQT segments are moved together)
    // (the restart markers at the very end, with nothing after them, have always been dropped)
    $kept = (substr_count((string) $out, "\xff\xd0") >= (($name === 'restart markers') ? $n - 2 : 0))
         && (substr_count((string) $out, "\xff\xe1\x00\x02") === (($name === 'restart markers') ? 0 : $n / 2))
         && (substr_count((string) $out, "\xff\xdb\x00\x02") >= (($name === 'restart markers') ? 0 : $n / 2));
    echo "$name: replace=", var_export($r, true), " segments kept=", var_export($kept, true),
         // it took a minute before, and takes a few hundredths of a second now
         " in time=", var_export($elapsed < 10, true), "\n";
    // the same result as a JPEG that is given again
    $o2 = new SWFEditor();
    $o2->input(file_get_contents('tests/colorformat.swf'));
    $o2->replaceJpegData(4, $out);
    echo "  stable=", var_export($o2->getJpegData(4) === $out, true), "\n";
}
?>
===DONE===
--EXPECT--
restart markers: replace=true segments kept=true in time=true
  stable=true
APP1 and DQT segments: replace=true segments kept=true in time=true
  stable=true
===DONE===
