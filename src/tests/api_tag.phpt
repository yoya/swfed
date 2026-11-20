--TEST--
getTagDetail(), get/replaceTagDataByCID(), get/replaceTagContentsByCID(), removeTag(), printTagData()
--FILE--
<?php
$neg = file_get_contents('tests/negimiku.swf');
$o = new SWFEditor();
$o->input($neg);
$before = $o->output();

echo "* getTagDetail\n";
foreach (array(2, 3, 5, 9) as $i) {
    echo $i, ": ", json_encode($o->getTagDetail($i)), "\n";
}
$d = $o->getTagDetail(10); // PlaceObject2
echo "10: character_id=", $d['character_id'], " depth=", $d['depth'], "\n";

echo "* TagDataByCID\n";
$data = $o->getTagDataByCID(2); // DefineShape (tag 3)
var_dump($data === $o->getTagData(3));
var_dump($o->getTagDataByCID(100));
var_dump($o->replaceTagDataByCID(2, $data));
var_dump($o->output() === $before);

echo "* TagContentsByCID\n";
$contents = $o->getTagContentsByCID(2); // after the CharacterId
var_dump($contents === substr($data, 2 + 2));
var_dump($o->replaceTagContentsByCID(2, $contents));
$jpeg = $o->getTagContentsByCID(1);    // DefineBitsJPEG
var_dump(strlen($jpeg), bin2hex(substr($jpeg, 0, 2)));
var_dump($o->replaceTagContentsByCID(1, $jpeg));
var_dump($o->output() === $before);

echo "* removeTag\n";
$n = count($o->getTagList());
var_dump($o->removeTag(4)); // PlaceObject2
var_dump(count($o->getTagList()) === $n - 1);
var_dump($o->removeTag(8, 0)); // the first tag in DefineSprite
var_dump($o->removeTag(100));

echo "* printTagData\n";
ob_start();
var_dump($o->printTagData($o->getTagData(2)));
$ret = ob_get_clean();
echo substr($ret, strrpos($ret, 'bool(')); // the printed tag may come before or after it
?>
===DONE===
--EXPECTF--
* getTagDetail
2: {"image_id":1,"jpeg_data_len":14769}
3: {"shape_id":2,"fill_styles.count":2,"line_styles.count":0,"bitmap_ref":[1]}
5: {"image_id":3,"jpeg_data_len":13423,"alpha_data_len":52122}
9: {"sprite_id":7,"frame_count":19}
10: character_id=7 depth=2
* TagDataByCID
bool(true)
%Abool(false)
bool(true)
bool(true)
* TagContentsByCID
bool(true)
bool(true)
int(14769)
string(4) "ffd8"
bool(true)
bool(true)
* removeTag
bool(true)
bool(true)
bool(true)
%Abool(false)
* printTagData
%Abool(true)
%A===DONE===%A
