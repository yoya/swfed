--TEST--
broken SWF / arguments do not crash (the v2.1 fixes)
--FILE--
<?php
function tag($code, $body) {
    $len = strlen($body);
    if ($len < 0x3f) {
        return pack('v', ($code << 6) | $len) . $body;
    }
    return pack('vV', ($code << 6) | 0x3f, $len) . $body;
}
function swf($version, $tags) {
    $body = "\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr($version) . pack('V', 8 + strlen($body)) . $body;
}
// input, then every tag through getTagDetail() / getTagData(), rebuild() and output()
function check($name, $swf) {
    $o = new SWFEditor();
    $ret = $o->input($swf);
    if ($ret) {
        foreach ($o->getTagList() as $i => $t) {
            $o->getTagDetail($i);
            $o->getTagData($i);
        }
        $o->rebuild();
        $o->output();
    }
    echo $name, ": ", var_export($ret, true), "\n";
    return $o;
}
$head = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');

echo "* header\n";
check('empty', '');
check('magic only', 'FWS');
check('unknown magic', 'XWS' . substr(swf(6, $end), 3));
check('huge RECT', "FWS\x06" . pack('V', 21) . "\xf8\xff\xff");
check('CWS impossible file_length', "CWS\x06" . pack('V', 0xffffffff) . gzcompress('x'));
check('CWS broken zlib', "CWS\x06" . pack('V', 100) . 'not zlib data');

echo "* tags\n";
check('tag longer than the data', swf(6, $head . pack('vV', (2 << 6) | 0x3f, 0x7fffffff) . 'abc'));
// DefineShape: 255 fill styles (extended count) with no data
check('shape styles cut', swf(6, $head . tag(2, pack('v', 1) . "\x00" . "\xff\xff\x00") . $end));
// DefineShape: a style change record with MoveTo bits running out
check('shape records cut', swf(6, $head . tag(2, pack('v', 1) . "\x00" . "\x00\x00\x11" . "\x0b\xff") . $end));
// DoAction: Push claims 255 bytes
check('action cut', swf(6, $head . tag(12, "\x96\xff\x00\x00") . $end));
// DefineButton2: a BUTTONRECORD without the end flag
check('button cut', swf(6, $head . tag(34, pack('v', 1) . "\x00" . pack('v', 0) . "\x0f" . pack('vv', 1, 1)) . $end));
// DefineEditText: flags say there is text but no string follows
check('edit text cut', swf(6, $head . tag(37, pack('v', 1) . "\x00" . "\x80\x00") . $end));

echo "* images\n";
$o = check('lossless broken zlib', swf(6, $head . tag(20, pack('vCvv', 1, 5, 10, 10) . 'not zlib') . $end));
var_dump($o->getPNGData(1));
$o = check('lossless too small', swf(6, $head . tag(20, pack('vCvv', 1, 5, 1000, 1000) . gzcompress("\x00\x00\x00\x00")) . $end));
var_dump($o->getPNGData(1));
$o = check('JPEG3 alpha offset too large', swf(6, $head . tag(35, pack('vV', 1, 0x7fffffff) . "\xff\xd8\xff\xd9") . $end));
var_dump($o->getJpegAlpha(1));
$o = check('JPEG broken', swf(6, $head . tag(21, pack('v', 1) . "\xff\xd8\xff\xc0\x00") . $end));
var_dump($o->getBitmapSize(1));
var_dump($o->replaceBitmapData(1, "\x89PNG\r\n\x1a\n" . 'broken'));
var_dump(is_bool($o->replaceBitmapData(1, "\xff\xd8\xff\xc0\x00")));

echo "* arguments\n";
$o = new SWFEditor();
$o->input(file_get_contents('tests/negimiku.swf'));
var_dump($o->setHeaderInfo(array('version' => '6', 'compress' => 'yes')));
var_dump($o->replaceBitmapData(array('width' => '320', 'height' => '240'), file_get_contents('tests/5.jpg'), '', array('rgb15' => '1')));
var_dump($o->getTagDetail(-1));
var_dump($o->getTagData(1000000));

echo "* not constructed\n";
$r = new ReflectionClass('SWFEditor');
$o = $r->newInstanceWithoutConstructor();
try {
    $ret = $o->output();
    echo "returned ", var_export($ret, true), "\n";
} catch (Error $e) {
    echo get_class($e), "\n";
}
?>
===DONE===
--EXPECTF--
* header
%Aempty: false
%Amagic only: false
%Aunknown magic: false
%Ahuge RECT: %s
%ACWS impossible file_length: false
%ACWS broken zlib: false
* tags
%Atag longer than the data: %s
%Ashape styles cut: %s
%Ashape records cut: %s
%Aaction cut: %s
%Abutton cut: %s
%Aedit text cut: %s
* images
%Alossless broken zlib: %s
%Abool(false)
%Alossless too small: %s
%Abool(false)
%AJPEG3 alpha offset too large: %s
%Abool(false)
%AJPEG broken: %s
%Abool(false)
%Abool(false)
%Abool(true)
* arguments
bool(true)
bool(true)
%Abool(false)
%Abool(false)
* not constructed
%A%r(Error|returned false)%r
===DONE===
