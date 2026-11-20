--TEST--
truncated or randomly broken SWF files do not crash
--FILE--
<?php
$files = array('4color-300.swf', 'colorformat.swf', 'colorformat_trans.swf',
               'font_test.swf', 'font_test_material.swf', 'negimiku.swf',
               'negimiku2_mcnest.swf', 'saitama.swf', 'saitama3.swf',
               'textvar-no0.swf', 'textvar-noaction.swf', 'textvar.swf');
mt_srand(70);
$count = 0;
$ok = 0;
foreach ($files as $file) {
    $swf = file_get_contents("tests/$file");
    $len = strlen($swf);
    $cases = array();
    for ($i = 0 ; $i < 40 ; $i++) {
        $cases[] = substr($swf, 0, mt_rand(0, $len)); // truncated
    }
    for ($i = 0 ; $i < 40 ; $i++) {
        $broken = $swf;
        for ($j = 0 ; $j < 4 ; $j++) {
            $broken[mt_rand(8, $len - 1)] = chr(mt_rand(0, 255)); // broken bytes
        }
        $cases[] = $broken;
    }
    foreach ($cases as $data) {
        $o = new SWFEditor();
        if ($o->input($data)) {
            foreach ($o->getTagList() as $i => $t) {
                $o->getTagDetail($i);
                $o->getTagData($i);
            }
            $o->rebuild();
            $o->output();
            $ok++;
        }
        $count++;
    }
}
echo "$count inputs\n";
var_dump($ok > 0);
?>
===DONE===
--EXPECTF--
%A960 inputs
bool(true)
===DONE===
