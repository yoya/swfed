--TEST--
isBitmapTag()
--FILE--
<?php

$swfdata = file_get_contents('tests/saitama.swf');
$obj = new SWFEditor();
$obj->input($swfdata);
foreach ($obj->getTagList() as $seqno => $tagInfo) {
    $data = $obj->getTagData($seqno);
    if ($obj->isBitmapTagData($data)) {
	echo "seqno=$seqno\n";
	var_export($tagInfo);
	echo "\n";
    }
}
?>
===DONE===
--EXPECTF--
seqno=3
array (
  'tag' => 6,
  'tagName' => 'DefineBitsJPEG',
  'length' => 2133,
  'detail' => true,
)
===DONE===
