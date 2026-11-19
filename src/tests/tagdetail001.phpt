--TEST--
getTagDetail() keys have no trailing NUL
--FILE--
<?php

$obj = new SWFEditor();
$obj->input(file_get_contents('tests/textvar.swf'));
foreach ($obj->getTagList() as $i => $tag) {
    if ($tag['code'] == 37) { // DefineEditText
        $d = $obj->getTagDetail($i);
        echo json_encode(array_keys($d)), "\n";
        var_dump(isset($d['variable_name']));
    }
}

$obj->input(file_get_contents('tests/negimiku2_mcnest.swf'));
$d = $obj->getTagDetail(14); // PlaceObject2 with name
echo json_encode(array_keys($d)), "\n";
var_dump(isset($d['name']));
?>
===DONE===
--EXPECTF--
["edit_id","variable_name"%A]
bool(true)
["character_id","depth","name"]
bool(true)
===DONE===
