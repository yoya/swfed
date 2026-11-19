--TEST--
replaceTagData() / replaceTagDataByCID() / replaceShapeData() on the first and last tag
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

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/saitama.swf'));
$before = $obj->output();
$last = count($obj->getTagList()) - 1;
var_dump($obj->replaceTagData(0, $obj->getTagData(0)));
var_dump($obj->replaceTagData($last, $obj->getTagData($last)));
var_dump($obj->replaceTagData(1, $obj->getTagData(1)));
var_dump($obj->output() === $before);

// SWF whose first tag is a definition tag (DefineShape id=2, empty shape)
$shape = pack('v', 2) . "\x00" . "\x00\x00" . "\x00\x00";
$obj->input(swf(4, tag(2, $shape) . tag(1, '') . tag(0, '')));
var_dump($obj->replaceTagDataByCID(2, $obj->getTagDataByCID(2)));
var_dump($obj->replaceShapeData(2, $obj->getShapeData(2)));
$obj2 = new SWFEditor();
var_dump($obj2->input($obj->output()));
echo implode(',', array_map(function ($t) { return $t['code']; },
                            $obj2->getTagList())), "\n";
?>
===DONE===
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
2,1,0
===DONE===
