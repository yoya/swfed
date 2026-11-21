--TEST--
applyShapeRectFactor() / applyShapeMatrixFactor() refuse values that don't fit, and change nothing (#153)
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
function bits($s) { // "0101..." to bytes
    $s .= str_repeat('0', (8 - strlen($s) % 8) % 8);
    return implode('', array_map(function ($b) { return chr(bindec($b)); }, str_split($s, 8)));
}
function rect($x_max, $y_max) { // (0, 0)-(x_max, y_max) twips, Nbits = 16
    return bits(sprintf('%05b%016b%016b%016b%016b', 16, 0, $x_max, 0, $y_max));
}
// DefineShape 1: solid fill, a line from (0, 0) to (1000, 0) twips
function shape_solid() {
    $records = '0' . '00001' . sprintf('%05b', 1) . '0' . '0'     // setup: MoveTo(0, 0), 1 bit each
             . '1' . '1' . sprintf('%04b', 10) . '0' . '0' . sprintf('%012b', 1000) // straight edge, horizontal
             . '000000';                                          // end
    return tag(2, pack('v', 1) . rect(2000, 2000) . "\x01" . "\x00\xff\x00\x00" . "\x00" . "\x10" . bits($records));
}
// DefineShape 2: clipped bitmap fill of bitmap 9 with an empty matrix, no edges
function shape_bitmap() {
    return tag(2, pack('v', 2) . rect(2000, 2000) . "\x01" . "\x41" . pack('v', 9) . "\x00"
                  . "\x00" . "\x10" . "\x00");
}

function call($method, $args) {
    $o = new SWFEditor();
    $o->input(swf(6, shape_solid() . shape_bitmap() . tag(1, '') . tag(0, '')));
    $before = $o->getShapeData($args[0]);
    error_clear_last();
    $r = @call_user_func_array(array($o, $method), $args);
    $e = error_get_last();
    $warning = $e ? preg_replace('/^.*\(\): /', '', $e['message']) : null;
    $after = $o->getShapeData($args[0]);
    $o2 = new SWFEditor();
    $ok = $o2->input($o->output()) && $o2->rebuild();
    echo var_export($r, true), ($after === $before) ? ' unchanged' : ' changed', $ok ? '' : ' BROKEN OUTPUT',
         $warning ? " ($warning)" : '', "\n";
}

echo "== applyShapeRectFactor\n";
call('applyShapeRectFactor', array(1, 2.0, 2.0, 10, 10));
call('applyShapeRectFactor', array(1, 1e9, 1e9, 0, 0));
call('applyShapeRectFactor', array(1, 1.0, 1e300, 0, 0));
call('applyShapeRectFactor', array(1, -1e9, 1.0, 0, 0));
call('applyShapeRectFactor', array(1, NAN, 1.0, 0, 0));
call('applyShapeRectFactor', array(1, 1.0, INF, 0, 0));
call('applyShapeRectFactor', array(1, 1.0, 1.0, 200000000, 0));
call('applyShapeRectFactor', array(1, 1.0, 1.0, 0, 1 << 40));
call('applyShapeRectFactor', array(1, 1.0, 1.0, 53687091, 0));  // the largest move, but the rect is 100 pixels wide
call('applyShapeRectFactor', array(1, 1.0, 1.0, -53687091, 0));
call('applyShapeRectFactor', array(1, 200.0, 1.0, 0, 0));       // the edge gets too long for an edge record
echo "== applyShapeMatrixFactor\n";
call('applyShapeMatrixFactor', array(2, 2.0, 2.0, 0.5, 10, 10));
call('applyShapeMatrixFactor', array(2, 16000.0, 1.0, 0.0, 0, 0));
call('applyShapeMatrixFactor', array(2, 20000.0, 1.0, 0.0, 0, 0));
call('applyShapeMatrixFactor', array(2, 1.0, 1e9, 0.0, 0, 0));
call('applyShapeMatrixFactor', array(2, 16000.0, 16000.0, 0.7, 0, 0));
call('applyShapeMatrixFactor', array(2, NAN, 1.0, 0.0, 0, 0));
call('applyShapeMatrixFactor', array(2, 1.0, 1.0, INF, 0, 0));
call('applyShapeMatrixFactor', array(2, 1.0, 1.0, 0.0, 0, -53687092));
call('applyShapeMatrixFactor', array(2, 1.0, 1.0, 0.0, 53687091, 53687091));
?>
===DONE===
--EXPECT--
== applyShapeRectFactor
true changed
false unchanged
false unchanged
false unchanged
false unchanged (scale_x must be a finite number)
false unchanged (scale_y must be a finite number)
false unchanged (trans_x(200000000) must be between -53687091 and 53687091)
false unchanged (trans_y(1099511627776) must be between -53687091 and 53687091)
false unchanged
true changed
false unchanged
== applyShapeMatrixFactor
true changed
true changed
false unchanged
false unchanged
true changed
false unchanged (scale_x must be a finite number)
false unchanged (rotate_rad must be a finite number)
false unchanged (trans_y(-53687092) must be between -53687091 and 53687091)
true changed
===DONE===
