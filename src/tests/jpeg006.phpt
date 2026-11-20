--TEST--
replaceJpegData() keeps a whole JPEG with restart markers (DRI / RSTn) or fill bytes before EOI
--FILE--
<?php

function swf($version, $tags) {
    $body = "\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr($version) . pack('V', 8 + strlen($body)) . $body;
}
$swfdata = file_get_contents('tests/saitama.swf');

// replaceJpegData() -> output() -> input() -> getJpegData()
function roundtrip($swfdata, $jpeg) {
    $o = new SWFEditor();
    $o->input($swfdata);
    if (! $o->replaceJpegData(1, $jpeg)) {
        return false;
    }
    $o2 = new SWFEditor();
    $o2->input($o->output());
    return $o2->getJpegData(1);
}
function same($out, $jpeg) {
    if ($out === false) {
        return 'failed';
    }
    return ($out === $jpeg) ? 'same' : sprintf('%d bytes (expected %d)', strlen($out), strlen($jpeg));
}

// a baseline JPEG with a restart marker after every MCU, and a progressive one
foreach (array('restart.jpg', 'restart_prog.jpg') as $file) {
    $jpeg = file_get_contents("tests/$file");
    echo $file, ": ", same(roundtrip($swfdata, $jpeg), $jpeg), "\n";
    // trailing data after EOI is still dropped
    echo $file, " with trailing data: ", same(roundtrip($swfdata, $jpeg . "trailing"), $jpeg), "\n";
}

// fill bytes (0xFF) may precede the EOI marker; they are not part of the image data
$jpeg = file_get_contents('tests/5.jpg');
$fill = substr($jpeg, 0, -2) . "\xff\xff\xff\xd9";
$out = roundtrip($swfdata, $fill);
echo "fill bytes before EOI: ", ($out !== false) ? 'kept' : 'failed', " ",
     (($out !== false) && (substr($out, -2) === "\xff\xd9") && (strlen($out) >= strlen($jpeg))) ? 'complete' : 'cut', "\n";
?>
===DONE===
--EXPECT--
restart.jpg: same
restart.jpg with trailing data: same
restart_prog.jpg: same
restart_prog.jpg with trailing data: same
fill bytes before EOI: kept complete
===DONE===
