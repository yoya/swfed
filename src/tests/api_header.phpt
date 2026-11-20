--TEST--
getHeaderInfo(), setHeaderInfo(), setCompressLevel(), rebuild(), _destroy_and_exit()
--FILE--
<?php
function header_of($swf) {
    $o = new SWFEditor();
    $o->input($swf);
    $h = $o->getHeaderInfo();
    return sprintf("compress=%s version=%d rect=(%d,%d)-(%d,%d)",
                   var_export($h['compress'], true), $h['version'],
                   $h['x_min'], $h['y_min'], $h['x_max'], $h['y_max']);
}
$neg = file_get_contents('tests/negimiku.swf');
echo header_of($neg), "\n";

echo "* setHeaderInfo\n";
$o = new SWFEditor();
$o->input($neg);
var_dump($o->setHeaderInfo(array('version' => '8'))); // the other keys stay
echo header_of($o->output()), "\n";
var_dump($o->setHeaderInfo(array('compress' => 1, 'x_max' => 320, 'y_max' => 200)));
$out = $o->output();
var_dump(substr($out, 0, 4) === "CWS\x08");
echo header_of($out), "\n";
var_dump($o->setHeaderInfo(array('compress' => false)));
echo header_of($o->output()), "\n";
var_dump($o->setHeaderInfo(array()));

echo "* setCompressLevel\n";
$o->setHeaderInfo(array('compress' => true));
var_dump($o->setCompressLevel(1));
$out1 = $o->output();
var_dump($o->setCompressLevel(9));
$out9 = $o->output();
var_dump(strlen($out9) < strlen($out1));
$o1 = new SWFEditor();
$o1->input($out1);
$o1->setHeaderInfo(array('compress' => false));
$o9 = new SWFEditor();
$o9->input($out9);
$o9->setHeaderInfo(array('compress' => false));
var_dump($o1->output() === $o9->output());

echo "* rebuild\n";
$o = new SWFEditor();
$o->input($neg);
$list = $o->getTagList();
var_dump($o->rebuild());
$o2 = new SWFEditor();
var_dump($o2->input($o->output()));
var_dump(count($o2->getTagList()) === count($list));
var_dump($o2->getJpegData(1) === $o->getJpegData(1));

echo "* _destroy_and_exit\n";
var_dump($o->_destroy_and_exit()); // kept for compatibility, does nothing
var_dump(strlen($o->output()) > 0);
?>
===DONE===
--EXPECT--
compress=false version=4 rect=(0,0)-(240,240)
* setHeaderInfo
bool(true)
compress=false version=8 rect=(0,0)-(240,240)
bool(true)
bool(true)
compress=true version=8 rect=(0,0)-(320,200)
bool(true)
compress=false version=8 rect=(0,0)-(320,200)
bool(true)
* setCompressLevel
bool(true)
bool(true)
bool(true)
bool(true)
* rebuild
bool(true)
bool(true)
bool(true)
bool(true)
* _destroy_and_exit
bool(true)
bool(true)
===DONE===
