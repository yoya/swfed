--TEST--
swfInfo() goes through the output buffer, errors go to getLastError() and swfed.error_output
--INI--
swfed.error_output=none
--FILE--
<?php
var_dump(ini_get('swfed.error_output'));
$o = new SWFEditor();
var_dump(SWFEditor::getLastError());
var_dump($o->input(file_get_contents('tests/saitama.swf')));

echo "* swfInfo\n";
ob_start();
$ret = $o->swfInfo();
$info = ob_get_clean();
var_dump($ret);
echo substr($info, 0, strpos($info, "\n")), "\n";
var_dump(substr_count($info, "\n") > 80);

echo "* getLastError\n";
var_dump($o->getShapeData(1)); // DefineBitsJPEG
var_dump(SWFEditor::getLastError());
var_dump($o->getHeaderInfo()['version']); // clears
var_dump(SWFEditor::getLastError());
var_dump($o->input('FWS'));
var_dump(SWFEditor::getLastError());

echo "* warning\n";
ini_set('swfed.error_output', 'warning');
var_dump($o->input('FWS'));

echo "* stderr\n";
// the message goes to the C stderr, which run-tests puts in the output of the
// test (so it is not caught by ob_start()); the name is case insensitive
ini_set('swfed.error_output', 'StdErr');
ob_start();
$ret = $o->input('FWS');
$captured = ob_get_clean();
var_dump($ret, $captured);
var_dump(SWFEditor::getLastError());

echo "* stderr keeps the order with the print output\n";
function tag($code, $body) {
    $len = strlen($body);
    return pack('v', ($code << 6) | $len) . $body;
}
// a DefineShape whose detail can't be parsed: swfInfo() prints errors after the tag line
$body = "\x00" . pack('vv', 12 << 8, 1) . tag(9, "\xff\xff\xff")
      . tag(2, pack('v', 1) . "\x00" . "\xff\xff\x00") . tag(1, '') . tag(0, '');
$o->input('FWS' . chr(6) . pack('V', 8 + strlen($body)) . $body);
$o->swfInfo();
?>
===DONE===
--EXPECTF--
string(0) ""
NULL
bool(true)
* swfInfo
bool(true)
magic=FWS  version=4  file_length=2704
bool(true)
* getLastError
bool(false)
string(96) "swf_object_get_shapedata: not isShapeTag(6)
getShapeData: Can't swf_object_get_shapedata (cid=1)"
int(4)
NULL
bool(false)
string(37) "swf_header_parse: input too short (3)"
* warning

Warning: SWFEditor::input(): swf_header_parse: input too short (3) in %s on line %d
bool(false)
* stderr
swf_header_parse: input too short (3)
bool(false)
string(0) ""
string(37) "swf_header_parse: input too short (3)"
* stderr keeps the order with the print output
magic=FWS  version=6  file_length=%d
 rect=(0, 0)-(0, 0) (f_size=0)
frame_rate=12.0  frame_count=1
[0] tag=SetBackgroundColor(9)  length=3
[1] tag=DefineShape(2)  length=6
swf_fill_style_parse: unknown fill_style->type=0xff
swf_fill_style_array_parse: swf_fill_style_parse failed i=0
swf_styles_parse: swf_fill_style_array_parse failed
swf_shape_with_style_parse: swf_styles_parse failed
ERROR: swf_tag_shape_input_detail: swf_shape_with_style_parse swf_tag_shape->shape_with_style failed. shape_id=1
swf_tag_create_input_detail: can't input tag detail (result=1)
swf_tag_print: swf_tag_create_input_detail failed
[2] tag=ShowFrame(1)
[3] tag=End(0)
===DONE===
