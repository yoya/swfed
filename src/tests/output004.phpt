--TEST--
swfed.error_output is case insensitive and rejects unknown values
--INI--
swfed.error_output=none
--FILE--
<?php
// where does the error go: "none" (getLastError() only) or "warning"
function where($o) {
    $warnings = array();
    set_error_handler(function ($no, $str) use (&$warnings) { $warnings[] = $str; return true; });
    $o->input('FWS');
    restore_error_handler();
    $kept = (SWFEditor::getLastError() !== null) ? 'kept' : 'lost';
    return $kept . ', ' . (count($warnings) ? 'warning' : 'no warning');
}

$o = new SWFEditor();
echo "* values\n";
foreach (array('none', 'None', 'OFF', 'off', '', '0', 'false', 'No',
               'warning', 'Warning', 'WARNING') as $value) {
    $old = ini_set('swfed.error_output', $value);
    printf("%-10s accepted=%s  %s\n", var_export($value, true),
           var_export($old !== false, true), where($o));
}

echo "* unknown values\n";
ini_set('swfed.error_output', 'warning');
foreach (array('bogus', 'on', 'true', '1', 'stdout', 'warnings') as $value) {
    $warnings = array();
    set_error_handler(function ($no, $str) use (&$warnings) { $warnings[] = $str; return true; });
    $old = ini_set('swfed.error_output', $value);
    restore_error_handler();
    printf("%-10s ini_set=%s  kept=%s  %s\n", var_export($value, true),
           var_export($old, true), ini_get('swfed.error_output'), $warnings[0]);
}
echo where($o), "\n"; // the value is kept

echo "* ini_restore\n";
ini_restore('swfed.error_output');
var_dump(ini_get('swfed.error_output'));
echo where($o), "\n";
?>
===DONE===
--EXPECT--
* values
'none'     accepted=true  kept, no warning
'None'     accepted=true  kept, no warning
'OFF'      accepted=true  kept, no warning
'off'      accepted=true  kept, no warning
''         accepted=true  kept, no warning
'0'        accepted=true  kept, no warning
'false'    accepted=true  kept, no warning
'No'       accepted=true  kept, no warning
'warning'  accepted=true  kept, warning
'Warning'  accepted=true  kept, warning
'WARNING'  accepted=true  kept, warning
* unknown values
'bogus'    ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "bogus" (stderr, warning or none)
'on'       ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "on" (stderr, warning or none)
'true'     ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "true" (stderr, warning or none)
'1'        ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "1" (stderr, warning or none)
'stdout'   ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "stdout" (stderr, warning or none)
'warnings' ini_set=false  kept=warning  ini_set(): Invalid swfed.error_output value "warnings" (stderr, warning or none)
kept, warning
* ini_restore
string(0) ""
kept, no warning
===DONE===
