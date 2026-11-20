--TEST--
replaceActionStrings() also replaces onClipEvent actions of PlaceObject3
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
function act($id, $data = null) {
    return ($id < 0x80) ? chr($id) : chr($id) . pack('v', strlen($data)) . $data;
}
$trace = act(0x96, "\x00LONGSTRING\0") . act(0x26) . act(0x00);

// CLIPACTIONS (SWF 6+): onClipEvent(load), and on(keyPress "<Space>") with a KeyCode
function clip_actions($trace) {
    return pack('v', 0) . pack('V', 0x01 | 0x020000)
         . pack('V', 0x01) . pack('V', strlen($trace)) . $trace
         . pack('V', 0x020000) . pack('V', 1 + strlen($trace)) . "\x20" . $trace
         . pack('V', 0);
}
// FILTERLIST: DropShadow, Blur, ColorMatrix, Convolution 2x2, GradientGlow with 2 colors
$filters = "\x05"
    . "\x00" . str_repeat("\x11", 23)
    . "\x01" . str_repeat("\x22", 9)
    . "\x06" . str_repeat("\x33", 80)
    . "\x05" . "\x02\x02" . str_repeat("\x44", 13 + 4 * 2 * 2)  // MatrixX, MatrixY, then 13 + 4 * x * y bytes
    . "\x04" . "\x02" . str_repeat("\x55", 5 * 2 + 19);           // NumColors, then 5 * n + 19 bytes

// the fields of PlaceObject3 before CLIPACTIONS
$head_min = "\x82\x00" . pack('v', 1) . pack('v', 2);                  // HasClipActions, HasCharacter: Depth, CharacterId
$head_all = "\xfe\x6f" . pack('v', 1)                                  // all but Move; filter, blend, cache, class name, visible, opaque bg
          . "Cls\0" . pack('v', 2)                                     // ClassName, CharacterId
          . "\x00"                                                     // Matrix (identity)
          . "\x00"                                                     // CXFORMWITHALPHA (no terms)
          . pack('v', 9) . "mc\0" . pack('v', 5)                       // Ratio, Name, ClipDepth
          . $filters . "\x07" . "\x01" . "\x01" . "\x00\x11\x22\xff";  // filters, BlendMode, BitmapCache, Visible, BackgroundColor
$head_img = "\x86\x10" . pack('v', 1) . "Img\0" . pack('v', 2) . "\x00"; // HasClipActions, HasMatrix, HasCharacter + HasImage: ClassName comes with HasImage and HasCharacter

$heads = array('minimal' => $head_min, 'all fields' => $head_all, 'HasImage and HasCharacter' => $head_img);

// output() without any replacement (it writes the header again)
function plain_output($swf) {
    $o = new SWFEditor();
    $o->input($swf);
    return $o->output();
}

function codes_data($o, $code) {
    foreach ($o->getTagList() as $i => $t) {
        if ($t['code'] == $code) {
            return $o->getTagData($i);
        }
    }
}

foreach ($heads as $name => $head) {
    $body = $head . clip_actions($trace);
    $swf = swf(8, tag(9, "\xff\xff\xff") . tag(70, $body) . tag(1, '') . tag(0, ''));
    $o = new SWFEditor();
    $o->input($swf);
    var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
    $d = codes_data($o, 70);
    $new_body = substr($d, ((ord($d[0]) & 0x3f) == 0x3f) ? 6 : 2);
    echo $name, ": LONGSTRING=", substr_count($d, 'LONGSTRING'), " S=", substr_count($d, "\x00S\x00"),
         " head kept=", (substr($new_body, 0, strlen($head)) === $head) ? "yes" : "no",
         " shorter=", (strlen($new_body) < strlen($body)) ? "yes" : "no", "\n";
    // the output is a valid SWF and rebuilds to the same bytes
    $out = $o->output();
    $o2 = new SWFEditor();
    var_dump($o2->input($out));
    var_dump($o2->rebuild());
    var_dump($o2->output() === $out);
    // a second pass finds nothing more to replace
    var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
    var_dump($o->output() === $out);
}

echo "== inside a DefineSprite\n";
$sprite = tag(39, pack('vv', 3, 1) . tag(70, $head_all . clip_actions($trace)) . tag(1, '') . tag(0, ''));
$o = new SWFEditor();
$o->input(swf(8, tag(9, "\xff\xff\xff") . $sprite . tag(1, '') . tag(0, '')));
var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
$out = $o->output();
echo "LONGSTRING=", substr_count($out, 'LONGSTRING'), " S=", substr_count($out, "\x00S\x00"), "\n";
$o2 = new SWFEditor();
var_dump($o2->input($out));
var_dump($o2->rebuild());
var_dump($o2->output() === $out);

echo "== nothing to replace, or no clip actions\n";
foreach (array(
    'no HasClipActions' => tag(70, "\x02\x00" . pack('v', 1) . pack('v', 2)),
    'other strings'     => tag(70, $head_min . clip_actions($trace)),
) as $name => $place) {
    $swf = swf(8, tag(9, "\xff\xff\xff") . $place . tag(1, '') . tag(0, ''));
    $o = new SWFEditor();
    $o->input($swf);
    var_dump($o->replaceActionStrings(array($name == 'other strings' ? 'nothing' : 'LONGSTRING' => 'S')));
    echo $name, ": unchanged=", ($o->output() === plain_output($swf)) ? "yes" : "no", "\n";
}

echo "== broken clip actions\n";
$clip = clip_actions($trace);
foreach (array(
    'truncated'        => substr($clip, 0, -9),
    'size too large'   => substr($clip, 0, 6 + 4) . pack('V', 0x7fffffff) . substr($clip, 6 + 8),
    'no clip actions'  => '',
) as $name => $broken) {
    $swf = swf(8, tag(9, "\xff\xff\xff") . tag(70, $head_all . $broken) . tag(1, '') . tag(0, ''));
    $o = new SWFEditor();
    $o->input($swf);
    var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
    echo $name, ": unchanged=", ($o->output() === plain_output($swf)) ? "yes" : "no", "\n";
}
?>
===DONE===
--EXPECTF--
bool(true)
minimal: LONGSTRING=0 S=2 head kept=yes shorter=yes
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
all fields: LONGSTRING=0 S=2 head kept=yes shorter=yes
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
HasImage and HasCharacter: LONGSTRING=0 S=2 head kept=yes shorter=yes
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
== inside a DefineSprite
bool(true)
LONGSTRING=0 S=2
bool(true)
bool(true)
bool(true)
== nothing to replace, or no clip actions
bool(true)
no HasClipActions: unchanged=yes
bool(true)
other strings: unchanged=yes
== broken clip actions
%Abool(false)
truncated: unchanged=yes
%Abool(false)
size too large: unchanged=yes
%Abool(false)
no clip actions: unchanged=yes
===DONE===
