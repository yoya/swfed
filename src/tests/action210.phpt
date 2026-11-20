--TEST--
replaceActionStrings() fixes branch offsets and function sizes when lengths change
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
function push_str($s) { return act(0x96, "\x00" . $s . "\0"); }

// function f() { trace("LONGSTRING"); }  if (true) { trace("LONGSTRING"); }  trace("END");
$body = push_str('LONGSTRING') . act(0x26);
$block = push_str('LONGSTRING') . act(0x26);
$actions = act(0x9B, "f\0" . pack('vv', 0, strlen($body))) . $body
         . act(0x96, "\x05\x01") . act(0x12) // Push true, Not
         . act(0x9D, pack('v', strlen($block))) . $block
         . push_str('END') . act(0x26) . act(0x00);
$swf = swf(6, tag(9, "\xff\xff\xff") . tag(12, $actions) . tag(1, '') . tag(0, ''));

// walk the actions: start offsets, and where codeSize / BranchOffset point to
function check($data) {
    $starts = array();
    $refs = array();
    $pos = 0;
    while ($pos < strlen($data)) {
        $id = ord($data[$pos]);
        $starts[] = $pos;
        $len = ($id < 0x80) ? 0 : unpack('v', substr($data, $pos + 1, 2))[1];
        $end = $pos + (($id < 0x80) ? 1 : 3 + $len);
        $d = substr($data, $pos + 3, $len);
        if ($id == 0x9B) {
            $refs[] = 'codeSize->' . ($end + unpack('v', substr($d, -2))[1]);
        } elseif ($id == 0x9D || $id == 0x99) {
            $refs[] = 'branch->' . ($end + unpack('s', substr($d, 0, 2))[1]);
        } elseif ($id == 0x94) {
            $refs[] = 'with->' . ($end + unpack('v', substr($d, 0, 2))[1]);
        } elseif ($id == 0x8F) {
            $s = unpack('vtry/vcatch/vfinally', substr($d, 1, 6));
            $refs[] = 'try->' . ($end + $s['try']);
            $refs[] = 'catch->' . ($end + $s['try'] + $s['catch']);
            $refs[] = 'finally->' . ($end + $s['try'] + $s['catch'] + $s['finally']);
        }
        $pos = $end;
    }
    $starts[] = $pos;
    $ok = true;
    foreach ($refs as $r) {
        $ok = $ok && in_array((int) explode('->', $r)[1], $starts, true);
    }
    return ($ok ? 'boundaries ok' : 'BROKEN') . ' ' . implode(' ', $refs);
}

$o = new SWFEditor();
foreach (array('S', str_repeat('X', 100)) as $to) {
    $o->input($swf);
    echo "before: ", check($o->getActionData(1)), "\n";
    var_dump($o->replaceActionStrings(array('LONGSTRING' => $to)));
    $o2 = new SWFEditor();
    $o2->input($o->output());
    $data = $o2->getActionData(1);
    echo "after:  ", check($data), "\n";
    var_dump(substr_count($data, $to . "\0"));
}

// with (o) { trace("LONGSTRING"); }  try { .. } catch (e) { .. } finally { .. }
$blk = push_str('LONGSTRING') . act(0x26);
$actions = push_str('o') . act(0x1C) . act(0x94, pack('v', strlen($blk))) . $blk
         . act(0x8F, "\x03" . pack('vvv', strlen($blk), strlen($blk), strlen($blk)) . "e\0")
         . $blk . $blk . $blk . act(0x00);
$o->input(swf(7, tag(9, "\xff\xff\xff") . tag(12, $actions) . tag(1, '') . tag(0, '')));
echo "before: ", check($o->getActionData(1)), "\n";
var_dump($o->replaceActionStrings(array('LONGSTRING' => 'S')));
$o2 = new SWFEditor();
$o2->input($o->output());
echo "after:  ", check($o2->getActionData(1)), "\n";
?>
===DONE===
--EXPECT--
before: boundaries ok codeSize->25 branch->52
bool(true)
after:  boundaries ok codeSize->16 branch->34
int(2)
before: boundaries ok codeSize->25 branch->52
bool(true)
after:  boundaries ok codeSize->115 branch->232
int(2)
before: boundaries ok with->28 try->56 catch->72 finally->88
bool(true)
after:  boundaries ok with->19 try->38 catch->45 finally->52
===DONE===
