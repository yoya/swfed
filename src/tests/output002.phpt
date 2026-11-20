--TEST--
output buffer callbacks and error handlers can use the same SWFEditor while swfInfo() is running
--INI--
swfed.error_output=none
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
$head = tag(9, "\xff\xff\xff");
$end = tag(1, '') . tag(0, '');
// every DefineShape fails to parse its detail: swfInfo() reports an error for each
$broken = swf(6, $head . str_repeat(tag(2, pack('v', 1) . "\x00" . "\xff\xff\x00"), 20) . $end);
$saitama = file_get_contents('tests/saitama.swf');
$other = file_get_contents('tests/textvar.swf');
$other_tags = new SWFEditor();
$other_tags->input($other);
$other_tags = count($other_tags->getTagList());

function info($o) {
    ob_start();
    $o->swfInfo();
    return ob_get_clean();
}

echo "* print: ob_start callback calls input() of the same object\n";
$o = new SWFEditor();
$o->input($saitama);
$expected = info($o);
$buf = '';
$called = 0;
ob_start(function ($b) use ($o, $other, &$buf, &$called) {
    $buf .= $b;
    $called++;
    $o->input($other);
    return '';
}, 1);
$ret = $o->swfInfo();
ob_end_clean();
var_dump($ret);
var_dump($called > 0);
var_dump(strlen($buf) > 1000);
var_dump($buf === $expected);
var_dump(count($o->getTagList()) === $other_tags);

echo "* print: ob_start callback throws\n";
$o->input($saitama);
ob_start(); // takes what the failed callback does not
ob_start(function ($b) { throw new Exception("from the callback"); }, 1);
$msg = "no exception";
try {
    $o->swfInfo();
} catch (Exception $e) {
    $msg = get_class($e) . ": " . $e->getMessage();
}
ob_end_clean();
ob_end_clean();
echo $msg, "\n";
var_dump(count($o->getTagList()) > 1);

echo "* warning: set_error_handler calls input() of the same object\n";
ini_set('swfed.error_output', 'warning');
$o = new SWFEditor();
var_dump($o->input($broken));
$count = 0;
set_error_handler(function ($no, $str) use (&$count) { $count++; return true; });
$expected = info($o);
restore_error_handler();
$expected_count = $count;
var_dump($expected_count >= 20);
$count = 0;
$buf = '';
set_error_handler(function ($no, $str) use ($o, $other, $broken, &$count) {
    $count++;
    $o->input(($count % 2) ? $other : $broken);
    return true;
});
ob_start(function ($b) use (&$buf) { $buf .= $b; return ''; });
$ret = $o->swfInfo();
ob_end_clean();
restore_error_handler();
var_dump($ret);
var_dump($count === $expected_count);
var_dump($buf === $expected);

echo "* warning: set_error_handler throws\n";
$o->input($broken);
set_error_handler(function ($no, $str) { throw new ErrorException($str, 0, $no); });
ob_start();
try {
    $o->swfInfo();
    echo "no exception\n";
} catch (ErrorException $e) {
    $output = ob_get_clean();
    echo get_class($e), ": ", $e->getMessage(), "\n";
}
restore_error_handler();
var_dump(strlen(SWFEditor::getLastError()) > 100); // the messages are kept
var_dump($o->getHeaderInfo()['version']);          // and the object is alive
echo "alive\n";
?>
===DONE===
--EXPECT--
* print: ob_start callback calls input() of the same object
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
* print: ob_start callback throws
Exception: from the callback
bool(true)
* warning: set_error_handler calls input() of the same object
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
* warning: set_error_handler throws
ErrorException: SWFEditor::swfInfo(): swf_fill_style_parse: unknown fill_style->type=0xff
bool(true)
int(6)
alive
===DONE===
