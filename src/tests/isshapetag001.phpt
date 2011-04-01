--TEST--
isShapeTag()
--FILE--
<?php

$swfdata = file_get_contents('tests/saitama.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
foreach ($obj->getTagList() as $seqno => $tagInfo) {
    $data = $obj->getTagData($seqno);
    if ($obj->isShapeTagData($data)) {
	echo "seqno=$seqno\n";
	var_export($tagInfo);
	echo "\n";
    }
}
?>
===DONE===
--EXPECTF--
seqno=4
array (
  'tag' => 2,
  'tagName' => 'DefineShape',
  'length' => 51,
  'detail' => true,
)
===DONE===
