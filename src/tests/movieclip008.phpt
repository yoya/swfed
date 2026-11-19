--TEST--
replaceMovieClip() purges unused contents when unused_cid_purge is true
--FILE--
<?php

$a = file_get_contents('tests/negimiku.swf');
$b = file_get_contents('tests/saitama3.swf');

function replaced($a, $b, $args) {
    $o = new SWFEditor();
    $o->input($a);
    $r = call_user_func_array(array($o, 'replaceMovieClip'), array_merge(array('miku', $b), $args));
    return array($r, $o);
}

list($r, $o) = replaced($a, $b, array());
var_dump($r);
$plain = $o->output();
$o->purgeUselessContents();
$purged = $o->output();
var_dump(strlen($purged) < strlen($plain));

list($r, $o) = replaced($a, $b, array(true));
var_dump($r, $o->output() === $purged);
list($r, $o) = replaced($a, $b, array(false));
var_dump($r, $o->output() === $plain);
list($r, $o) = replaced($a, $b, array(null));
var_dump($r, $o->output() === $plain);
?>
===DONE===
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
===DONE===
