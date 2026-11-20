--TEST--
setActionVariables() / replaceActionStrings() / purgeUselessContents() report failure
--FILE--
<?php

function tag($code, $body) {
    $len = strlen($body);
    if ($len < 0x3f) {
        return pack('v', ($code << 6) | $len) . $body;
    }
    return pack('vV', ($code << 6) | 0x3f, $len) . $body;
}

// before input()
$obj = new SWFEditor();
var_dump($obj->setActionVariables(array('a' => 'b')));

// broken DoAction (Push length runs past the end of the tag)
$obj->input(file_get_contents('tests/textvar.swf'));
$obj->replaceTagData(1, tag(12, "\x96\x05\x00AB"));
var_dump($obj->replaceActionStrings(array('ABCDEFG' => 'X')));

var_dump($obj->purgeUselessContents());
?>
===DONE===
--EXPECTF--
%Abool(false)
%Abool(false)
bool(true)
===DONE===
