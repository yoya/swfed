--TEST--
exit() in an error handler called for the output of swfInfo() does not leak or crash
--INI--
swfed.error_output=warning
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
$broken = swf(6, $head . str_repeat(tag(2, pack('v', 1) . "\x00" . "\xff\xff\x00"), 20) . $end);

$o = new SWFEditor();
$o->input($broken);
register_shutdown_function(function () use ($o) {
    // the object still works after the exit, and nothing is left over
    echo "shutdown: ", var_export($o->getHeaderInfo()['version'], true), "\n";
});
set_error_handler(function ($no, $str) {
    ob_end_clean();
    echo "handler\n";
    exit(0);
});
ob_start(function ($b) { return ''; }); // the dump is not the point
$o->swfInfo();
echo "not reached\n";
?>
===DONE===
--EXPECT--
handler
shutdown: 6
