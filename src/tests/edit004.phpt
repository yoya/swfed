--TEST--
DefineEditText keeps WasStatic / AutoSize and parses HasFontClass
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
// flags byte 2 of the DefineEditText in the output
function flags2($o) {
    $n = new SWFEditor();
    $n->input($o->output());
    foreach ($n->getTagList() as $i => $t) {
        if ($t['code'] == 37) {
            $d = $n->getTagData($i);
            $pos = (((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2) + 2;   // header, CharacterID
            $pos += (int) ceil((5 + 4 * (ord($d[$pos]) >> 3)) / 8); // RECT
            return sprintf('0x%02x', ord($d[$pos + 1]));
        }
    }
}

// HasText|HasFont, AutoSize|WasStatic, FontID=1, FontHeight=240
$edit = tag(37, pack('v', 2) . "\x00" . "\x81\x44" . pack('vv', 1, 240) . "msg\0" . "hello\0");
$o = new SWFEditor();
$o->input(swf(8, tag(9, "\xff\xff\xff") . $edit . tag(1, '') . tag(0, '')));
var_dump($o->replaceEditString('msg', 'world'));
echo flags2($o), "\n";
// a detail made without the SWF object (purge) must not lose AutoSize
$o->input(swf(8, tag(9, "\xff\xff\xff") . $edit . tag(1, '') . tag(0, '')));
$o->purgeUselessContents();
var_dump($o->replaceEditString('msg', 'world'));
echo flags2($o), "\n";

// HasText, HasFontClass|AutoSize, FontClass="MyFont", FontHeight=240
$edit = tag(37, pack('v', 2) . "\x00" . "\x80\xc0" . "MyFont\0" . pack('v', 240) . "msg\0" . "hello\0");
$o->input(swf(9, tag(9, "\xff\xff\xff") . $edit . tag(1, '') . tag(0, '')));
var_dump($o->getEditString('msg'));
var_dump($o->replaceEditString('msg', 'world'));
echo flags2($o), "\n";
var_dump(strpos($o->output(), "MyFont\0" . pack('v', 240) . "msg\0world\0") !== false);
?>
===DONE===
--EXPECT--
bool(true)
0x44
bool(true)
0x44
string(5) "hello"
bool(true)
0xc0
bool(true)
===DONE===
