<?php

/*
 * アップロードされた SWF や取り出した画像は、呼び出し元ユーザー専用の
 * ディレクトリ `<sys_get_temp_dir()>/swfed-<uid>/` に保存する。
 * 0700 で作成し、他ユーザーからの先行配置（シンボリックリンク攻撃など）
 * を防ぐ。古いファイルは `prune_tmp_dir()` で一定時間経過後に削除する。
 */
function swfed_tmp_dir() {
    $base = sys_get_temp_dir();
    $uid  = function_exists('posix_geteuid')? posix_geteuid(): 0;
    $dir  = rtrim($base, '/') . '/swfed-' . $uid;
    if (! is_dir($dir)) {
        @mkdir($dir, 0700, true);
    }
    @chmod($dir, 0700);
    return $dir;
}

function swfed_prune_tmp_dir($dir, $ttl_seconds = 3600) {
    $now = time();
    $dh = @opendir($dir);
    if (! $dh) { return; }
    while (($name = readdir($dh)) !== false) {
        if ($name === '.' || $name === '..') { continue; }
        $path = $dir . '/' . $name;
        if (is_link($path) || ! is_file($path)) {
            @unlink($path);
            continue;
        }
        $mtime = @filemtime($path);
        if ($mtime !== false && ($now - $mtime) > $ttl_seconds) {
            @unlink($path);
        }
    }
    closedir($dh);
}

$tmp_dir    = swfed_tmp_dir();
$tmp_prefix = $tmp_dir . '/swf-';
swfed_prune_tmp_dir($tmp_dir);

function dec_from_string($n) {
    return preg_match('/^([0-9]+)/', ltrim($n), $m)? $m[0]: "";
}

function hex_from_string($h) {
    return preg_match('/^([0-9a-fA-F]+)/', ltrim($h), $m)? $m[0]: "";
}

// echo dec_from_string("  11aaa44").PHP_EOL;
// echo hex_from_string(" 22aa<a").PHP_EOL;
