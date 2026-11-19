<?php

$tmp_prefix = '/tmp/swf-';

function dec_from_string($n) {
    return preg_match('/^([0-9]+)/', ltrim($n), $m)? $m[0]: "";
}

function hex_from_string($h) {
    return preg_match('/^([0-9a-fA-F]+)/', ltrim($h), $m)? $m[0]: "";
}

// echo dec_from_string("  11aaa44").PHP_EOL;
// echo hex_from_string(" 22aa<a").PHP_EOL;

if (! function_exists('multiply_unit')) {
    function multiply_unit($str) {
        $str = preg_replace_callback('/(\d+)G/i', function($m) {
                return $m[1]*1024*1024*1024; }, $str);
        $str = preg_replace_callback('/(\d+)M/i', function($m) {
                return $m[1]*1024*1024; }, $str);
        $str = preg_replace_callback('/(\d+)K/i', function($m) {
                return $m[1]*1024; }, $str);
        $str = preg_replace_callback('/(\d+)B/i', function($m) {
                return $m[1]; } , $str);
        return $str;
    }
}
