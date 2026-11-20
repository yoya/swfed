--TEST--
LZMA compressed SWF (ZWS) input / output
--SKIPIF--
<?php
ob_start();
phpinfo(INFO_MODULES);
if (strpos(ob_get_clean(), 'LZMA (ZWS) support => yes') === false) {
    die('skip swfed built without liblzma');
}
?>
--FILE--
<?php
$fws = file_get_contents('tests/saitama.swf');
$zws = file_get_contents('tests/saitama_lzma.swf'); // saitama.swf re-compressed with LZMA

$ref = new SWFEditor();
$ref->input($fws);
$o = new SWFEditor();
var_dump($o->input($zws));
$h = $o->getHeaderInfo();
var_dump($h['compress'], $h['lzma'], $h['version']);
var_dump($o->getTagList() == $ref->getTagList());
// LZMA data without the end marker
$o3 = new SWFEditor();
var_dump($o3->input(file_get_contents('tests/saitama_lzma_noeopm.swf')));
var_dump($o3->getTagList() == $ref->getTagList());

// output keeps ZWS, and it reads back to the same movie
$out = $o->output();
echo substr($out, 0, 3), "\n";
var_dump(strlen($out) < strlen($fws));
$o2 = new SWFEditor();
var_dump($o2->input($out));
$o2->setHeaderInfo(array('compress' => false));
var_dump($o2->output() === $ref->output());
// every compress level (the LZMA preset) gives a ZWS that reads back to the same movie
foreach (array(-1, 0, 9) as $level) {
    $o->setCompressLevel($level);
    $n = new SWFEditor();
    var_dump($n->input($o->output()) && $n->getTagList() == $ref->getTagList());
}

// setHeaderInfo()
function magic($o) { return substr($o->output(), 0, 3); }
$o = new SWFEditor();
$o->input($fws);
var_dump($o->setHeaderInfo(array('lzma' => true)));
echo magic($o), "\n";
$o->setHeaderInfo(array('compress' => true));    // stays ZWS
echo magic($o), "\n";
$o->setHeaderInfo(array('lzma' => false));       // ZWS -> CWS
echo magic($o), "\n";
$o->setHeaderInfo(array('compress' => true, 'lzma' => true));
echo magic($o), "\n";
$h = $o->getHeaderInfo();
$h['compress'] = false;                          // like sample/swfcompress.php -u
$o->setHeaderInfo($h);
echo magic($o), "\n";

// broken ZWS
$o = new SWFEditor();
var_dump($o->input(substr($zws, 0, 1000)));                    // truncated
var_dump($o->input(substr($zws, 0, 16)));                      // no LZMA data
var_dump($o->input(substr_replace($zws, "\xff", 12, 1)));      // bad properties
var_dump($o->input(substr_replace($zws, "\xff\xff\xff\xff", 4, 4))); // file_length too large
var_dump($o->input($zws));
echo count($o->getTagList()), "\n";
?>
===DONE===
--EXPECTF--
bool(true)
bool(true)
bool(true)
int(4)
bool(true)
bool(true)
bool(true)
ZWS
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
ZWS
ZWS
CWS
ZWS
FWS
%Abool(false)
%Abool(false)
%Abool(false)
%Abool(false)
bool(true)
85
===DONE===
