--TEST--
input() fails on a truncated or broken SWF
--FILE--
<?php

function tag($code, $body) {
    $len = strlen($body);
    if ($len < 0x3f) {
        return pack('v', ($code << 6) | $len) . $body;
    }
    return pack('vV', ($code << 6) | 0x3f, $len) . $body;
}
// $extra: bytes claimed by file_length but missing from the data
function swf($version, $tags, $extra = 0) {
    $body = "\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr($version) . pack('V', 8 + strlen($body) + $extra) . $body;
}

$swf = file_get_contents('tests/saitama.swf');
$o = new SWFEditor();
// cut in the middle of a tag
var_dump($o->input(substr($swf, 0, 1352)));
// cut at a tag boundary, before file_length
var_dump($o->input(swf(6, tag(9, "\xff\xff\xff") . tag(1, ''), 10)));
// compressed data that inflates to less than file_length
$body = substr(swf(6, tag(9, "\xff\xff\xff") . tag(1, '')), 8);
var_dump($o->input("CWS\x06" . pack('V', 8 + strlen($body) + 10) . gzcompress($body)));

// complete data without an End tag is still accepted
var_dump($o->input(swf(6, tag(9, "\xff\xff\xff") . tag(1, ''))));
echo count($o->getTagList()), "\n";
var_dump($o->input($swf));
echo count($o->getTagList()), "\n";
?>
===DONE===
--EXPECTF--
%Abool(false)
%Abool(false)
%Abool(false)
bool(true)
2
bool(true)
85
===DONE===
