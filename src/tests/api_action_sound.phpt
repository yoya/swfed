--TEST--
getActionData(), disasmActionData(), getSoundData(), replaceMLDData()
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

echo "* getActionData\n";
$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
foreach ($o->getTagList() as $i => $t) {
    if ($t['code'] == 12) { // DoAction
        $action = $o->getActionData($i);
        echo $i, ": ", bin2hex($action), "\n"; // GotoFrame(1) Play End
    }
}
var_dump($o->getActionData(0)); // SetBackgroundColor

echo "* getSoundData / replaceMLDData\n";
// DefineSound: uncompressed, 5.5kHz, 8 bits, mono, 4 samples
$swf = swf(6, tag(9, "\xff\xff\xff") . tag(14, pack('vCV', 1, 0x00, 4) . "\x80\x90\xa0\xb0")
              . tag(15, pack('v', 1) . "\x00") . tag(1, '') . tag(0, ''));
$o = new SWFEditor();
var_dump($o->input($swf));
echo bin2hex($o->getSoundData(1)), "\n";
var_dump($o->getSoundData(2));
$mld = "melo\x00\x00\x00\x10";
var_dump($o->replaceMLDData(1, $mld));
var_dump($o->getSoundData(1) === $mld);
$o2 = new SWFEditor();
var_dump($o2->input($o->output()));
var_dump($o2->getSoundData(1) === $mld);
$d = $o2->getTagDetail(1);
echo "format=", $d['format'], " sound_data_len=", $d['sound_data_len'], "\n";
var_dump($o->replaceMLDData(2, $mld));

// last: what it prints may come out of order
echo "* disasmActionData\n";
ob_start();
$ret = $o->disasmActionData($action);
ob_end_clean();
var_dump($ret);
?>
===DONE===
--EXPECTF--
* getActionData
81: 81020001000600
%Abool(false)
* getSoundData / replaceMLDData
bool(true)
8090a0b0
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
format=15 sound_data_len=8
bool(false)
* disasmActionData
%Abool(true)
%A===DONE===%A
