--TEST--
output() fails for a tag that can't be written out, instead of writing an empty tag (#149)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

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
// DefineShape2 with $n solid fill styles and no edges. More than 32767 can
// be read, but not written: NumFillBits is 4 bits, i.e. up to 15 bits per index
function shape($id, $n) {
    return tag(22, pack('v', $id) . "\x00" . "\xff" . pack('v', $n) . str_repeat("\x00\xff\x00\x00", $n)
                   . "\x00" . "\x10" . "\x00");
}
function error() {
    return (strpos((string) SWFEditor::getLastError(), 'too many styles') !== false) ? 'too many styles' : 'other';
}

foreach (array(32767, 40000) as $n) {
    echo "== $n fill styles\n";
    $data = swf(6, shape(1, $n) . tag(26, "\x02" . pack('vv', 1, 1)) . tag(1, '') . tag(0, ''));
    $o = new SWFEditor();
    var_dump($o->input($data));
    echo "as read: ", var_export($o->getTagData(0) === shape(1, $n), true), "\n";
    var_dump($o->rebuild());                      // the shape is parsed, its raw data is dropped
    $out = $o->output();
    if ($out === false) {
        echo "output: false (", error(), ")\n";
        echo "getTagData: ", var_export($o->getTagData(0), true), " (", error(), ")\n";
        echo "getTagDataByCID: ", var_export($o->getTagDataByCID(1), true), "\n";
        echo "getShapeData: ", var_export($o->getShapeData(1), true), "\n";
        echo "the other tags: ", strlen($o->getTagData(1)), "\n";
        try {
            $c = clone $o;
            echo "cloned\n";
        } catch (Error $e) {
            echo "clone: ", $e->getMessage(), "\n";
        }
    } else {
        $o2 = new SWFEditor();
        $o2->input($out);
        $list = $o2->getTagList();
        echo "output: shape length=", $list[0]['length'], "\n";
    }
}
?>
===DONE===
--EXPECT--
== 32767 fill styles
bool(true)
as read: true
bool(true)
output: shape length=131078
== 40000 fill styles
bool(true)
as read: true
bool(true)
output: false (too many styles)
getTagData: false (too many styles)
getTagDataByCID: false
getShapeData: false
the other tags: 7
clone: SWFEditor: failed to clone the SWF data
===DONE===
