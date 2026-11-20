<?php

/*
 * アップロードされた SWF や取り出した画像は、Web サーバーのユーザー専用の
 * ディレクトリに保存する。
 *
 * - 既定は `<sys_get_temp_dir()>/swfed-<euid>`。環境変数 SWFED_TMP_DIR で
 *   別の場所（Web から見えない、アプリ専用の場所など）を指定できる。
 * - 名前は予測できるので、/tmp を共有する他のユーザーが先に作っておける。
 *   使う前に lstat() で「シンボリックリンクではないディレクトリ」
 *   「所有者が自分」「パーミッションが 0700」であることを確かめ、
 *   満たさなければ処理を中断する。
 * - 古いファイルは swfed_prune_tmp_dir() で一定時間経過後に削除する。
 */

function swfed_tmp_fail($reason) {
    error_log("swfed: $reason");
    if (! headers_sent()) {
        http_response_code(500);
        header("Content-Type: text/plain; charset=utf-8");
    }
    echo "ごめんなさい。内部エラーです。（作業用ディレクトリを用意できません）\n";
    exit(1);
}

/* 実効 uid。posix 拡張が無ければ、新しく作った一時ファイルの所有者で代用する */
function swfed_euid() {
    if (function_exists('posix_geteuid')) {
        return posix_geteuid();
    }
    $probe = @tempnam(sys_get_temp_dir(), 'swfed');
    $uid = ($probe === false)? false: @fileowner($probe);
    if ($probe !== false) {
        @unlink($probe);
    }
    if ($uid === false) {
        swfed_tmp_fail("cannot determine the effective uid");
    }
    return $uid;
}

function swfed_tmp_dir() {
    // Windows では所有者とパーミッションを lstat() で確かめられないので、
    // ディレクトリであることだけを確かめる
    $posix = (DIRECTORY_SEPARATOR === '/');
    $euid  = $posix? swfed_euid(): null;
    $dir   = getenv('SWFED_TMP_DIR');
    if ($dir === false || $dir === '') {
        $dir = rtrim(sys_get_temp_dir(), '/\\') . '/swfed' . ($posix? "-$euid": '');
    }
    // 末尾に / があると lstat() がシンボリックリンクをたどってしまう
    $dir = rtrim($dir, '/\\');
    // 既にあれば失敗する。作れたかどうかに関係なく、以下で確かめる
    @mkdir($dir, 0700);
    clearstatcache(true, $dir);
    $st = ($dir === '')? false: @lstat($dir);
    if ($st === false) {
        swfed_tmp_fail("cannot create $dir");
    }
    if (($st['mode'] & 0170000) !== 0040000) {
        swfed_tmp_fail("$dir is not a directory (symbolic link or other file)");
    }
    if ($posix) {
        if ($st['uid'] !== $euid) {
            swfed_tmp_fail("$dir is owned by uid {$st['uid']}, not $euid");
        }
        if (($st['mode'] & 0777) !== 0700) {
            // 自分が所有する（リンクではない）ディレクトリなので絞り直してよい
            @chmod($dir, 0700);
            clearstatcache(true, $dir);
            $st = @lstat($dir);
            if ($st === false || ($st['mode'] & 0170777) !== 0040700) {
                swfed_tmp_fail("cannot change the mode of $dir to 0700");
            }
        }
    }
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
