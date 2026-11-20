--TEST--
Return values match the declared types; getTagList() detail flag matches getTagDetail()
--FILE--
<?php

$o = new SWFEditor();
$o->input(file_get_contents('tests/saitama.swf'));
var_dump(gettype($o->getTagData(1)));

$o->input(file_get_contents('tests/textvar.swf'));
$o->replaceEditString('textvar', '');
var_dump($o->getEditString('textvar'));
var_dump($o->getEditString('nope'));

$o->input(file_get_contents('tests/negimiku.swf'));
$list = $o->getTagList();
var_dump(isset($list[1]['detail']), $o->getTagDetail(1)); // JPEGTables

$bad = 0;
foreach (glob('tests/*.swf') as $file) {
    $o->input(file_get_contents($file));
    foreach ($o->getTagList() as $i => $tag) {
        $d = $o->getTagDetail($i);
        if (isset($tag['detail']) !== is_array($d)) {
            echo basename($file), " #$i code=", $tag['code'], " ", gettype($d), "\n";
            $bad++;
        }
        if (! is_array($d) && $d !== false) {
            echo basename($file), " #$i code=", $tag['code'], " returned ", gettype($d), "\n";
            $bad++;
        }
    }
}
echo "mismatch: $bad\n";
?>
===DONE===
--EXPECTF--
string(6) "string"
string(0) ""
%Abool(false)
bool(false)
bool(false)
mismatch: 0
===DONE===
