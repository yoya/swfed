--TEST--
DefineBitsJPEG3 / 4 whose alpha can't be inflated: the tag fails to parse instead of losing its alpha (#178)
--FILE--
<?php
ini_set('swfed.error_output', 'none');

function tag($code, $body) {
    return pack('vV', ($code << 6) | 0x3f, strlen($body)) . $body;
}
function swf($tags) {
    $body = "\x78\x00\x05\x5f\x00\x00\x0f\xa0\x00" . pack('vv', 12 << 8, 1) . $tags;
    return 'FWS' . chr(8) . pack('V', 8 + strlen($body)) . $body;
}
$jpeg = file_get_contents('tests/5.jpg');
$good = gzcompress(str_repeat("\x80", 400 * 300));
$cases = array(
    'good alpha'           => $good,
    'corrupt alpha'        => substr($good, 0, 20) . "\xff\xff\xff\xff" . substr($good, 24),
    'truncated alpha'      => substr($good, 0, 30),
    'alpha over the limit' => gzcompress(str_repeat("\x80", 1048576)),   // the limit is 512K
    'no alpha bytes'       => '',
);
ini_set('swfed.max_uncompressed_size', '512K');
foreach ($cases as $name => $alpha) {
    foreach (array(35 => 'JPEG3', 90 => 'JPEG4') as $code => $tagname) {
        $t = tag($code, pack('v', 1) . pack('V', strlen($jpeg)) . (($code == 90) ? "\x12\x34" : '') . $jpeg . $alpha);
        $o = new SWFEditor();
        $o->input(swf($t . "\x40\x00" . "\x00\x00"));
        $d = $o->getTagDetail(0);
        $rebuild = $o->rebuild();
        echo "$name ($tagname): detail=", is_array($d) ? ('alpha ' . (isset($d['alpha_data_len']) ? $d['alpha_data_len'] : 'none')) : var_export($d, true),
             " rebuild=", var_export($rebuild, true),
             " tag ", ($o->getTagData(0) === $t) ? 'as it was' : 'CHANGED',
             " jpeg=", var_export(is_string($o->getJpegData(1)), true),
             " alpha=", var_export(is_string($o->getJpegAlpha(1)), true), "\n";
    }
}
// the bitmap of a tag that can't be parsed can still be replaced
$o = new SWFEditor();
$o->input(swf(tag(35, pack('v', 1) . pack('V', strlen($jpeg)) . $jpeg . substr($good, 0, 30)) . "\x40\x00" . "\x00\x00"));
var_dump($o->replaceJpegData(1, $jpeg, str_repeat("\xff", 400 * 300)));
$d = $o->getTagDetail(0);
echo "replaced: alpha ", $d['alpha_data_len'], "\n";
?>
===DONE===
--EXPECT--
good alpha (JPEG3): detail=alpha 120000 rebuild=true tag as it was jpeg=true alpha=true
good alpha (JPEG4): detail=alpha 120000 rebuild=true tag as it was jpeg=true alpha=true
corrupt alpha (JPEG3): detail=false rebuild=false tag as it was jpeg=false alpha=false
corrupt alpha (JPEG4): detail=false rebuild=false tag as it was jpeg=false alpha=false
truncated alpha (JPEG3): detail=false rebuild=false tag as it was jpeg=false alpha=false
truncated alpha (JPEG4): detail=false rebuild=false tag as it was jpeg=false alpha=false
alpha over the limit (JPEG3): detail=false rebuild=false tag as it was jpeg=false alpha=false
alpha over the limit (JPEG4): detail=false rebuild=false tag as it was jpeg=false alpha=false
no alpha bytes (JPEG3): detail=alpha none rebuild=true tag as it was jpeg=true alpha=false
no alpha bytes (JPEG4): detail=alpha none rebuild=true tag as it was jpeg=true alpha=false
bool(true)
replaced: alpha 120000
===DONE===
