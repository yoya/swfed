# swfed - SWF Editor for PHP

[English](README.en.md) | 日本語

swfed は、Adobe Flash（SWF）ファイルを編集する PHP 拡張モジュールです。
`SWFEditor` というクラスを 1 つ提供し、文字列から SWF を読み込み、タグ単位で
調べたり編集したりして、文字列として書き出せます。

できること:

- SWF のヘッダ（バージョン、圧縮、フレームサイズ）の読み取りと変更
- タグの一覧、取り出し、置換、削除
- ビットマップを JPEG / PNG / GIF のデータで置換する。JPEG / PNG として取り出す
- テキストフィールド（DefineEditText）の初期テキストの編集
- ActionScript 1/2 のバイトコードの文字列の置換と、変数の注入
- ムービークリップの中身を別の SWF で置換する
- シェイプの拡大縮小や移動、使われていないシェイプやビットマップの削除

swfed が解釈できないタグ（たとえば ActionScript 3 の `DoABC`）は、そのまま
コピーされます。

## 必要なもの

- PHP 7.2 以降と、`phpize`（たいてい `php-dev` / `php-devel` パッケージに入っています）。
  CI は、Linux の PHP 7.2、8.0、8.3、8.4、8.5（UBSan と valgrind でも）と、
  Windows の PHP 8.3 で、ビルドとテストをしています。
- zlib（ヘッダ付き）。見つからないと `configure` は止まります。
- libpng、giflib、liblzma（xz）（ヘッダ付き）。これらは任意です。どれかが無くても
  `configure` は警告を出すだけで、その機能（PNG、GIF、LZMA 圧縮の `ZWS` ファイルの
  読み書き）なしでビルドします。
  - Debian / Ubuntu: `apt-get install zlib1g-dev libpng-dev libgif-dev liblzma-dev`
  - Homebrew: `brew install zlib libpng giflib xz`
- Unix 系 OS（Linux、macOS、*BSD）、または Windows（`src/config.w32`）。

## ビルドとインストール

```sh
git clone https://github.com/yoya/swfed.git
cd swfed/src
phpize
./configure
make
make test
sudo make install
```

そのあと、`php.ini`（または `conf.d` ディレクトリのファイル）で拡張モジュールを
有効にします。

```ini
extension=swfed
```

`php --ri swfed` で確認できます。バージョンと、PNG、GIF、LZMA（ZWS）の対応が
組み込まれているかが表示されます。

`configure` は、まず、オプションなしでコンパイラが見つけられるライブラリ（システムの
ディレクトリや macOS SDK）をそのまま使います。見つからなければ `pkg-config`、
続いて `/usr/local`、`/usr`、`/opt/homebrew`、`/opt/local` の下を探します。
ライブラリが別の場所にある場合は、そのインストール先（`include/` と `lib/` を含む
ディレクトリ）を指定してください。

| オプション | ライブラリ |
|---|---|
| `--with-zlib-dir=DIR` | zlib（`DIR/include/zlib.h`） |
| `--with-png-dir=DIR` | libpng（`DIR/include/png.h` または `DIR/include/libpng/png.h`） |
| `--with-gif-dir=DIR` | giflib（`DIR/include/gif_lib.h` または `DIR/include/giflib/gif_lib.h`） |
| `--with-lzma-dir=DIR` | liblzma（`DIR/include/lzma.h`） |

`--without-png-dir`、`--without-gif-dir`、`--without-lzma-dir` を指定すると、
そのライブラリなしでビルドします。

たとえば、Apple Silicon の Homebrew で、`./configure` だけでは見つからない場合:

```sh
./configure --with-zlib-dir=$(brew --prefix zlib) --with-png-dir=$(brew --prefix) \
            --with-gif-dir=$(brew --prefix) --with-lzma-dir=$(brew --prefix xz)
```

[doc/install.txt](doc/install.txt) も参照してください。

## かんたんな例

```php
<?php
$swf = new SWFEditor();
if (! $swf->input(file_get_contents('input.swf'))) {
    exit("input failed\n");
}
// キャラクター ID が 1 のビットマップを置換する（JPEG、PNG、GIF のデータ）
if (! $swf->replaceBitmapData(1, file_get_contents('new.jpg'))) {
    exit("replaceBitmapData failed\n");
}
file_put_contents('output.swf', $swf->output());
```

ID を調べるには、`php sample/swfdump.php input.swf` を実行するか、
`getTagList()` と `getTagDetail()` を使います。

## 基本

### タグインデックスとキャラクター ID

メソッドがタグを指す方法は 2 つあります。

- **タグインデックス**: 最上位のタグの並びの中での、0 から始まる位置です。
  `getTagList()` が返す順序に従います。`DefineSprite` の中のタグは数えません。
  タグを削除したり挿入したりすると、インデックスはずれます。
  `getTagDetail()`、`getTagData()`、`replaceTagData()`、`removeTag()`、
  `getActionData()` が使います。
- **キャラクター ID**（CID）: `Define*` タグの ID です（`getTagDetail()` の
  `image_id`、`shape_id`、`sound_id` など）。ID を受け取るそれ以外のメソッドが使います。
  最上位のタグだけが検索の対象です。

PHP のスタブでは、いくつかのビットマップ・サウンドのメソッド（`getJpegData()`、
`replacePNGData()`、`getSoundData()` など）で、ID の引数の名前が `$index` に
なっています。これはタグインデックスではなく、キャラクター ID です。名前付き引数で
呼ぶときは注意してください。

サイズと位置（`getHeaderInfo()`、`getBitmapSize()`、`trans_x` / `trans_y` 引数）は
ピクセル単位です。SWF は twip（1 ピクセル = 20 twip）で保存していて、
`getHeaderInfo()` は、値がピクセルの整数倍でないときは float を返します。

### 文字列と文字エンコーディング

swfed は文字エンコーディングを変換しません。渡す文字列も、受け取る文字列も
（編集テキスト、アクションの文字列と変数、インスタンス名、`getTagDetail()` の名前）、
SWF に保存されているバイト列そのままです。

SWF の仕様では、SWF バージョン 6 以降は文字列が UTF-8 で保存されます。SWF 5 以前は
作者のローカルなエンコーディングで、たとえば日本語なら Shift_JIS です（Flash Lite の
コンテンツに多く見られます）。バージョンを調べて、自分で変換してください。

```php
$info = $swf->getHeaderInfo();
$enc  = ($info['version'] >= 6) ? 'UTF-8' : 'SJIS-win';

echo mb_convert_encoding($swf->getEditString('title'), 'UTF-8', $enc);
$swf->replaceEditString('title', mb_convert_encoding('new title', $enc, 'UTF-8'));
```

`www/index.php` も、タグの詳細を表示するときに同じ規則（バージョン 6 未満は
`SJIS-win`）を使っています。
SWF の文字列は NUL で終わるため、新しい文字列に NUL バイトを入れると、そこで切れます。

### エラー

- ほとんどのメソッドは、失敗すると `false` を返します。SWF に関するエラーでは
  例外を投げません。
- C のコードは、問題をメッセージで説明します。たとえば
  `swf_header_parse: input too short (3)` のようなものです。
  `SWFEditor::getLastError()` は、直前のメソッド呼び出しのメッセージを返します。
  メッセージがほかにどこへ出るかは `swfed.error_output`（後述）で決まり、
  既定ではプロセスの stderr に書き出されます。これらは PHP の警告ではないので、
  `error_reporting()`、`@`、エラーハンドラからは見えません。
- 一部の失敗は、PHP の `E_WARNING` を出して `false` を返します。それらは、
  以下のメソッドの説明に書いてあります。負の、または 2147483647 を超えるタグ
  インデックスと、0〜65535 の範囲にないキャラクター ID は、どのメソッドでも
  そうなります。
- 引数の型や個数が違うと、PHP 8 では `TypeError` / `ArgumentCountError` を投げます。
  PHP 7 では警告を出し、メソッドは `false` または `null` を返します。
- `swfInfo()`、`printTagData()`、`disasmActionData()` は、PHP の出力
  （`echo` と同じ）を通して表示するので、`ob_start()` で取り込めます。
- 表示する内容と、`stderr` / `warning` 向けのメッセージは、メソッドがムービーの
  処理を終えたあとに、C のコードが出した順に書き出されます。処理の途中ではありません。
  そのため、出力バッファのコールバックやエラーハンドラの中から、同じ `SWFEditor` の
  メソッドを（`input()` でも）呼べます。そこで例外が投げられると、まだ書き出していない
  出力は捨てられます（`getLastError()` にはメッセージが残ります）。
- `replace*Data()` が失敗したとき、対象のビットマップタグは元のまま残ります。

#### `swfed.error_output`

C のコードのメッセージの出力先を決める ini 設定です（`PHP_INI_ALL`: php.ini、
`ini_set()` など）。

| 値 | メッセージの行き先 |
|---|---|
| `stderr`（既定） | プロセスの stderr |
| `warning` | PHP。メッセージごとに `E_WARNING` を 1 つ出す（`SWFEditor::method(): message`）ので、エラーハンドラから見える |
| `none` | どこにも出さない。`getLastError()` だけが持つ（`off` も同じ） |

値は大文字小文字を区別しません（`Off`、`Warning`）。php.ini の中の `Off` / `None` だけの
指定は空文字列になりますが、これも `none` として扱います。未知の値は `E_WARNING` を
出します。`ini_set()` は `false` を返して古い値のままにし、php.ini の間違った値は
`stderr` になります。

```php
ini_set('swfed.error_output', 'none');
if ($swf->input($data) === false) {
    error_log(SWFEditor::getLastError() ?? 'input() failed');
}
```

メソッドが引数について出す PHP の警告（たとえば範囲外のキャラクター ID）は、この
設定に関係なく、`getLastError()` にも入りません。

#### `swfed.max_uncompressed_size`

展開後のサイズの上限（バイト）を決める ini 設定です（`PHP_INI_ALL`）。ライブラリは
`malloc()` でメモリを確保するので、`memory_limit` の対象になりません。小さい入力が
大きなメモリを使わないように、次のそれぞれのサイズをこの値と比べます。

- 圧縮された SWF（`CWS` / `ZWS`）のヘッダにあるファイル長（`input()`、
  `replaceMovieClip()`）。超えると `E_WARNING` を出して `false` を返します。
- DefineBitsLossless / DefineBitsLossless2 の画像データと、DefineBitsJPEG3 /
  DefineBitsJPEG4 のアルファを展開したサイズ。超えると、そのタグの解析が失敗します
  （アルファは無いものとして扱います）。
- `replacePNGData()` / `replaceGIFData()` / `replaceBitmapData()` に渡した PNG / GIF を
  デコードしたサイズ（GIF は、論理画面と全フレームの合計）。超えると `false` を返します。

| 値 | 上限 |
|---|---|
| `0`（既定） | `memory_limit` の値。`memory_limit` が無制限なら、無制限 |
| 正の数（`64M` のような書き方も可） | その値 |
| `-1` | 無制限 |

### SWFEditor オブジェクト

- `new SWFEditor()` は、空のエディタを作ります。同じオブジェクトで
  `__construct()` をもう一度呼ぶと、`Error` を投げます。
- サブクラスのコンストラクタが `parent::__construct()` を呼ばなくても動きます。
- `clone` は、読み込んだ SWF と設定を含めて、独立したコピーを作ります。
- `serialize()` は `Exception` を投げます。代わりに `output()` の結果を
  シリアライズしてください。
- `input()` の前にもメソッドを呼べます。空のムービーに対して動きます
  （たとえば `output()` は `false` を返します）。

## API リファレンス

シグネチャは、実装が受け付けるものを示しています。省略可能な引数は
角括弧の中にあります。

### 入力、出力、ヘッダ

#### `input(string $data): bool`

SWF ファイルを解析し、それまでに読み込んでいたものと置き換えます。非圧縮
（`FWS`）、zlib 圧縮（`CWS`）、LZMA 圧縮（`ZWS`）のファイルに対応します。`ZWS` は
liblzma 付きのビルドが必要で、無いと `ZWS` ファイルでは `input()` が `false` を
返します（`php --ri swfed` で確認できます）。ヘッダを読めない、シグネチャが
`FWS` / `CWS` / `ZWS` ではない、展開に失敗した（`ZWS` の場合は、LZMA データの
破損や途中での切断、ヘッダのファイル長と合わないサイズ）ときは `false` を返し、
それまでに読み込んでいたムービーはそのまま残ります。
途中で切れたタグがある場合と、ヘッダのファイル長より前でデータが終わる場合も、
`false` を返します。最後の `End` タグが無いことだけは受け付けます。

#### `output(): string|false`

ムービーをシリアライズします。ファイル形式はヘッダに従い、`FWS`、`compress`
フラグが立っていれば `CWS`、さらに `lzma` も立っていれば `ZWS` になります
（読み込んだ `ZWS` は `ZWS` のままです）。圧縮には `setCompressLevel()` の
レベルを使います。ファイル長のフィールドは計算し直します。何も読み込んでいない
場合と、圧縮に失敗した場合、および liblzma なしのビルドで `ZWS` にする場合は、
`false` を返します。

#### `getHeaderInfo(): array`

次の値を返します。

| キー | 型 | 意味 |
|---|---|---|
| `compress` | bool | `CWS` と `ZWS` なら `true`、`FWS` なら `false` |
| `lzma` | bool | `ZWS`（LZMA）なら `true`、`FWS` と `CWS` なら `false` |
| `version` | int | SWF バージョン |
| `x_min`、`y_min`、`x_max`、`y_max` | int\|float | フレームの矩形（ピクセル。整数でないときは float） |

フレームレートとフレーム数は返しません。`input()` の前は、すべての値が
0 / `false` です。

#### `setHeaderInfo(array $header): bool`

`$header` にある `getHeaderInfo()` のキーを設定します。ほかのキーは無視します。
ファイル形式は `compress` と `lzma` から決まります。

| 指定 | 結果 |
|---|---|
| `compress => false` | `FWS`（`lzma => true` も一緒に指定しても） |
| `lzma => true` | `ZWS` |
| `compress => true`（`lzma` なし） | `CWS`。すでに `ZWS` のムービーは `ZWS` のまま |
| `lzma => false` | `ZWS` は `CWS` になる。`CWS` は `CWS` のまま。`FWS` は `FWS` のまま。ただし `compress => true` も一緒に指定すると `CWS` |

そのため、`getHeaderInfo()` の配列のキーを 1 つだけ変えて、そのまま渡せます。
`true` を返します。`version` が 0〜255 の範囲外、フレームの矩形の値が範囲外、
または liblzma なしのビルドで `lzma => true` を指定した場合は、`E_WARNING` を
出して `false` を返し、何も変更しません。配列でない引数はエラーです
（「エラー」を参照）。
`compress` を設定しても、swfed はバージョンを調整しません。圧縮したファイルには、
バージョン 6 以降（`CWS`）または 13 以降（`ZWS`）が必要です。
`sample/swfcompress.php` は、バージョンを 6 に上げます。

#### `setCompressLevel(int $level): bool`

`output()` が `CWS` ファイルに使う zlib のレベルです（`ZWS` ファイルでは LZMA の
プリセットで、`0`〜`9`、`-1` が LZMA の既定です）。swfed が作り直す
`DefineBitsJPEG3` / `DefineBitsJPEG4`（アルファ）と、`DefineBitsLossless` /
`DefineBitsLossless2` タグの中の zlib データにも使います。`-1`（zlib の既定で、
初期値）から `9` までです。範囲外だと、`E_WARNING` を出して `false` を返します。

#### `swfInfo(): bool`

ヘッダとすべてのタグを、読みやすい形で PHP の出力に表示します（上の「エラー」を
参照）。`ob_start()` で取り込めます。
常に `true` を返します。

#### `rebuild(): bool`

swfed が解釈できるすべてのタグを解析します。これで `output()` は、元のバイト列を
コピーする代わりに、解析したデータから書き出します。タグを解析できなければ
（そのタグで止まって）`false` を返します。出力は、たいてい入力とバイト単位では
一致しません。

#### `purgeUselessContents(): bool`

どの `PlaceObject` / `PlaceObject2` / `PlaceObject3` からも、直接にも、シェイプ・
ボタン・配置されたスプライトを経由しても使われておらず、書き出し（`ExportAssets` /
`SymbolClass`。たとえば `BitmapData.loadBitmap()` 用）もされていない、シェイプ
（`DefineShape`、`DefineShape2`、`DefineShape3`、`DefineShape4`、`DefineMorphShape`、
`DefineMorphShape2`）とビットマップ（`DefineBits*`）のタグを削除します。ほかの
キャラクターの種類（スプライト、フォント、テキスト、サウンドなど）は残ります。
`true` を返します。使われているシェイプ・スプライト・ボタン・配置タグの中に解析
できないものがあると、何を使っているかが分からないので、何も削除せずに `false` を
返します。

### タグ

#### `getTagList(): array`

最上位のタグごとに、ファイルの順に配列を 1 つ返します。

| キー | 型 | 意味 |
|---|---|---|
| `code` | int | タグの種類 |
| `tag` | int | `code` と同じ |
| `tagName` | string | タグの名前。swfed が知らない種類のタグには無い |
| `length` | int | タグヘッダを除いた、タグ本体のバイト長 |
| `detail` | bool | `getTagDetail()` がそのタグの種類を説明できれば `true`。できなければキー自体が無い |

#### `getTagDetail(int $index): array|false`

`$index` のタグを解析した情報を返します。タグの種類ごとのキーは次のとおりです。

| タグ（コード） | キー |
|---|---|
| DefineBitsJPEG (6)、DefineBitsJPEG2 (21)、DefineBitsJPEG3 (35)、DefineBitsJPEG4 (90) | `image_id`、`jpeg_data_len`、`alpha_data_len`（アルファのデータがあるときだけ） |
| DefineBitsLossless (20)、DefineBitsLossless2 (36) | `image_id`、`format`（3 = 8 ビットのカラーマップ、4 = 15 ビット RGB、5 = 32 ビット）、`width`、`height`、`colormap_count`（format 3 のときだけ） |
| DefineSound (14) | `sound_id`、`format`、`rate`、`is_16bits`、`is_stereo`、`sound_samples_count`、`sound_data_len`（SWF のフィールドの値そのまま） |
| DoAction (12)、DoInitAction (59) | `action_list_count`（空なら無い）、`action_sprite`（DoInitAction だけ） |
| DefineEditText (37) | `edit_id`、`variable_name` と `initial_text`（それぞれ、空でないときだけ） |
| DefineSprite (39) | `sprite_id`、`frame_count` |
| DefineShape (2)、DefineShape2 (22)、DefineShape3 (32)、DefineShape4 (83)、DefineMorphShape (46)、DefineMorphShape2 (84) | `shape_id`、`fill_styles.count`、`line_styles.count`、`bitmap_ref`（参照しているビットマップ ID のリスト。無ければ無い） |
| PlaceObject (4)、PlaceObject2 (26) | `character_id`（PlaceObject2 は、設定されているときだけ）、`depth`、`name`（設定されているときだけ） |

`$index` が範囲外の場合、タグの種類がこの表に無い場合（`getTagList()` に `detail` が
無い）、またはタグを解析できない場合は、`false` を返します。

#### `getTagData(int $index): string|false`

`$index` のタグ全体（タグヘッダと本体）を、`output()` が書き出すとおりに返します。
`$index` が範囲外なら `false` を返します。

#### `replaceTagData(int $index, string $data): bool`

`$index` のタグを `$data` で置き換えます。`$data` は、完全な 1 つのタグ（ヘッダと
本体。たとえば `getTagData()` の結果）でなければなりません。検査するのはタグ
ヘッダだけです。`$index` が範囲外か、`$data` が完全なタグでなければ `false` を
返します。

#### `getTagDataByCID(int $cid): string|false`

キャラクター `$cid` を定義しているタグ全体（ヘッダと本体）を返します。無ければ
`false` を返します。

#### `replaceTagDataByCID(int $cid, string $data): bool`

`$cid` を定義しているタグを `$data`（完全な 1 つのタグ）で置き換えます。`$data` の
中のキャラクター ID は `$cid` に書き換えられます。タグの種類は変わることがあります。
`$cid` が見つからないか、`$data` が完全なタグでなければ `false` を返します。

#### `getTagContentsByCID(int $cid): string|false`

`$cid` を定義しているタグの本体を、タグヘッダと、先頭の 2 バイトのキャラクター ID を
除いて返します。見つからなければ `false` を返します。

#### `replaceTagContentsByCID(int $cid, string $data): bool`

キャラクター ID より後ろの本体を置き換えます（`getTagContentsByCID()` の
対になるメソッドです）。タグの種類と ID は変わりません。`$data` は検査しません。
`$cid` が見つからなければ `false` を返します。

#### `removeTag(int $index [, ?int $index_in_sprite = null]): bool`

`$index` の最上位のタグを削除します。`$index_in_sprite` を指定すると、代わりに、
`$index` にある `DefineSprite` の中の、その位置のタグを削除します。`null` または
負の値は「指定なし」です。インデックスが範囲外のとき、または
`$index_in_sprite` を指定したのに `$index` のタグが `DefineSprite` ではないときは、
`false` を返します。（スタブでは、2 番目の引数の名前が `$index_sprint` に
なっています。）

#### `printTagData(string $data): bool`

完全な 1 つのタグ（たとえば `getTagData()` の結果）を、`swfInfo()` と同じ形式で、
PHP の出力に表示します。`$data` が完全なタグでなければ `false` を返します。

#### `isShapeTagData(string $data): bool`

`$data` が、`DefineShape`、`DefineShape2`、`DefineShape3`、`DefineShape4`、
`DefineMorphShape`、`DefineMorphShape2` のいずれかの、完全なタグであれば `true` を
返します。検査するのはタグヘッダだけです。
読み込んだムービーは使いません。

#### `isBitmapTagData(string $data): bool`

`isShapeTagData()` と同じで、対象は `DefineBitsJPEG`、`DefineBitsJPEG2`、
`DefineBitsJPEG3`、`DefineBitsJPEG4`、`DefineBitsLossless`、`DefineBitsLossless2` です。

### シェイプ

これらのメソッドは、`DefineShape`、`DefineShape2`、`DefineShape3`、`DefineShape4`、
および（取得と置換だけ）`DefineMorphShape`、`DefineMorphShape2` を扱います。

#### `getShapeData(int $cid): string|false`

シェイプのタグ `$cid` 全体（ヘッダと本体）を返します。`$cid` が見つからないか、
シェイプではなければ `false` を返します。

#### `replaceShapeData(int $cid, string $data): bool`

シェイプ `$cid` を `$data` で置き換えます。`$data` は、完全なシェイプのタグ
（たとえば `getShapeData()` の結果で、別の SWF のものでも構いません）です。
`$data` の中のシェイプ ID は `$cid` に書き換えられ、タグの種類は `$data` のものに
なります。新しいシェイプが参照しているビットマップ ID は、そのまま使われます。
`$data` がシェイプのタグでなければ、ID の無いシェイプの本体として読みます
（swfed 0.37 以前の形式です）。`$cid` がシェイプではないか、`$data` を解析できない
場合は、`false` を返します。

#### `applyShapeMatrixFactor(int $shape_id, float $scale_x, float $scale_y, float $rotate_rad, int $trans_x, int $trans_y): bool`

シェイプの中のすべてのビットマップ塗りとグラデーション塗りの、塗りの行列を
変換します。拡大縮小し、`$rotate_rad` ラジアン回転し、
（`$trans_x`、`$trans_y`）ピクセル移動します。輪郭は変わりません。その ID の
`DefineShape` / `2` / `3` が無ければ `false` を返します（`DefineMorphShape` は
サポートしていません）。結果が SWF の値の範囲（31 ビット）に収まらない場合も、何も
変更せずに `false` を返します。倍率や回転が有限の数でない場合と、移動量が
±53687091 ピクセルを超える場合は、`E_WARNING` も出します。

#### `applyShapeRectFactor(int $shape_id, float $scale_x, float $scale_y, int $trans_x, int $trans_y): bool`

シェイプの輪郭と境界を、左上の角を中心に拡大縮小し、
（`$trans_x`、`$trans_y`）ピクセル移動します。塗りの行列は変わりません。
`applyShapeMatrixFactor()` と同じ場合に `false` を返します。

#### `setShapeAdjustMode(int $mode): bool`

ビットマップを `replaceJpegData()`、`replacePNGData()`、`replaceGIFData()`、
`replaceBitmapData()` で置換したときに、そのビットマップを使うシェイプをどう調整
するかを設定します。モードはビットマスクで、変更するまで保たれ、`input()` の前に
設定してもかまいません。常に `true` を返します。値は検査しません。

| 定数 | 値 | 置換したビットマップを使う各シェイプへの効果 |
|---|---|---|
| `SWFEditor::SHAPE_BITMAP_NONE` | 0 | 何もしない（既定） |
| `SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE` | 1 | ビットマップ塗りの行列を、古いサイズ / 新しいサイズで拡大縮小して、新しい画像が古い画像と同じ領域を埋めるようにする |
| `SWFEditor::SHAPE_BITMAP_RECT_RESIZE` | 2 | シェイプの輪郭と境界を、新しいサイズ / 古いサイズで拡大縮小して、シェイプが新しい画像のサイズになるようにする |
| `SWFEditor::SHAPE_BITMAP_TYPE_TILLED` | 4 | クリップされたビットマップ塗りを、タイル（繰り返し）の塗りに変える |

値は `|` で組み合わせられます。サンプルは `SHAPE_BITMAP_RECT_RESIZE` を使っています。

#### `getShapeIdListByBitmapRef(int $bitmap_id): array`

塗りのスタイルがビットマップ `$bitmap_id` を使っているシェイプの ID を返します。
無ければ空の配列を返します。

### ビットマップ

置換のメソッドは、対象としてどのビットマップタグ（JPEG でもロスレスでも）でも
受け付け、タグの種類を新しいデータに合わせて変えます。置換に失敗したときに何が
残るかは、上の「エラー」を参照してください。

#### `getBitmapSize(int $bitmap_id): array|false`

`['width' => int, 'height' => int]` を、ピクセル単位で返します。`$bitmap_id` が
ビットマップではない場合や、JPEG のサイズを読めない場合は、`false` を返します。

#### `getJpegData(int $image_id): string|false`

`DefineBitsJPEG` / `2` / `3` / `4` タグの画像を、普通の JPEG ファイルとして返します
（`DefineBitsJPEG` では、共有の `JPEGTables` を組み込みます）。その ID の JPEG タグが
無ければ `false` を返します。

#### `getJpegAlpha(int $image_id): string|false`

`DefineBitsJPEG3` または `DefineBitsJPEG4` タグのアルファチャンネルを、展開して
返します。1 ピクセルにつき 1 バイトで、width × height バイトです。そのタグが
どちらでもない場合や、アルファのデータが無い場合は、`false` を返します。

#### `replaceJpegData(int $image_id, string $jpeg [, string $alpha]): bool`

ビットマップ `$image_id` を JPEG データで置き換えます。`$alpha` が無い（または
`''` の）場合、タグは `DefineBitsJPEG2` になります。`$alpha` がある場合は
`DefineBitsJPEG3` になります。すでに `DefineBitsJPEG4` だった場合は、
`DefineBitsJPEG4` のまま（`DeblockParam` を保ったまま）です。`$alpha` は、
`getJpegAlpha()` の形式の、展開されたアルファのデータで、swfed が圧縮します。
その ID のビットマップが無い場合と、`$jpeg` が JPEG でない場合（SOI が無い、
セグメントが壊れている、SOF / SOS が無い）は、`false` を返します。EOI より後ろの
データは捨てます。`$alpha` の長さは検査しません。`$alpha` に `null` を渡しても
動きますが、PHP 8.1 以降では `E_DEPRECATED` になるので、省略してください。

#### `getPNGData(int $image_id): string|false`

`DefineBitsLossless` / `2` タグを PNG ファイルとして返します（15 ビットカラーの
format 4 は、1 チャンネル 8 ビットになります）。ほかのタグでは `false` を返します。

#### `replacePNGData(int $image_id, string $png [, array $options]): bool`

ビットマップ `$image_id` を PNG 画像で置き換えます。

| PNG | 結果 |
|---|---|
| パレット | format 3。透明（tRNS）があれば `DefineBitsLossless2`、なければ `DefineBitsLossless` |
| RGB | `DefineBitsLossless`、format 5（`rgb15` では format 4） |
| RGB + アルファ | `DefineBitsLossless2`、format 5 |

グレースケールは RGB に変換し、1 チャンネル 16 ビットは 8 ビットに減らします。
グレースケールや RGB の PNG の透明（tRNS）は、アルファチャンネルになります。
オプションのキー:

- `rgb15`（bool）: RGB の PNG を 15 ビットカラー（format 4）で保存します。ほかの
  種類の PNG では、エラーメッセージ（「エラー」を参照）を出して無視します。

`$options` を指定するときは配列でなければならず、`null` は `TypeError` を
投げます。その ID のビットマップが無いか、PNG を変換できなければ `false` を
返します。

#### `replaceGIFData(int $image_id, string $gif): bool`

ビットマップ `$image_id` を、GIF ファイルの最初の画像で置き換えます
（`DefineBitsLossless`、format 3。GIF に透明色があれば `DefineBitsLossless2`）。
その ID のビットマップが無いか、GIF を読めなければ `false` を返します。

#### `replaceBitmapData(int|array $target, string $data [, string $alpha [, bool|array $options]]): bool`

`$data` が JPEG、PNG、GIF のどれかをシグネチャから判別して、
`replaceJpegData()`、`replacePNGData()`、`replaceGIFData()` と同じようにビットマップを
置き換えます。

`$target` は、キャラクター ID（配列でない値は int に変換されます）か、条件の
配列です。条件を指定した場合は、指定したキーのすべてに合う、ファイルの順で最初の
ビットマップタグが置換されます。

| キー | 一致の条件 |
|---|---|
| `width`、`height` | ピクセル単位のサイズが完全に一致 |
| `red`、`green`、`blue` | 最初のピクセルの色が、±10 の範囲で一致。ロスレスのビットマップだけが一致しうる |

`$alpha` は、JPEG データのときだけ使います。アルファのデータなしで `$options` を
渡すには、`$alpha` に `''` か `null` を渡します。

`$options` は、bool（`without_converting` と同じ）か、次のキーを持つ配列です。

- `without_converting`（bool）: ファイルのバイト列をそのまま `DefineBitsJPEG2`
  タグに保存します（JPEG、PNG、GIF。このタグの中の PNG と GIF は SWF 8 の機能です）。
  `$alpha` と `rgb15` は無視します。
- `rgb15`（bool）: `replacePNGData()` と同じです。

失敗すると `false` を返します。次の場合は `E_WARNING` も出します。画像の形式が
不明（"Unknown Bitmap Format"）、条件に合うビットマップが無い、キャラクター ID が
範囲外。

#### `convertBitmapDataToJpegTag(): bool`

最上位のすべての `DefineBitsLossless` / `2` タグを、画像を PNG として保持する
`DefineBitsJPEG2` タグに変換します（ロスレスで、表示には SWF 8 / Flash Player 8 が
必要です）。変換できないビットマップは変更しません。常に `true` を返します。

### サウンド

#### `getSoundData(int $sound_id): string|false`

`DefineSound` タグ `$sound_id` のサウンドデータを返します。タグのフォーマットの
フィールドより後ろのバイト列で、タグ自身のフォーマットのままです（MP3 では、
先頭の 2 バイトが SeekSamples フィールドです）。フォーマットとレートは
`getTagDetail()` で調べてください。見つからなければ `false` を返します。

#### `replaceMLDData(int $sound_id, string $mld): bool`

`DefineSound` `$sound_id` のデータを MLD（i モードのメロディ）のデータで置き換え、
サウンドのフォーマットを 15（Flash Lite が使うデバイスサウンド）、レートを 0、
8 ビット、モノラル、サンプル数を 0 にします。データは検査しません。見つからなければ
`false` を返します。

### テキストフィールド

#### `getEditString(string $name): string|null|false`

変数名が `$name` に完全に一致する、最初の `DefineEditText` の初期テキストを
返します。変数名が一致しなければ、`$name` を 10 進のキャラクター ID（たとえば
`"2"`）としても試します。一致したフィールドに初期テキストが無い（またはムービーを
読み込んでいない）場合は `null` を、どのフィールドにも一致しない場合は `false` を
返します。テキストは保存されているとおりに返します（「文字列と文字エンコーディング」を
参照）。

#### `replaceEditString(string $name, string $text): bool`

`$name` に一致するすべての `DefineEditText`（規則は `getEditString()` と同じ）の
初期テキストを設定します。`$text` が空だと、初期テキストを削除します。どの
フィールドにも一致しなければ `false` を返します。

### アクション（ActionScript 1/2）

#### `getActionData(int $index): string|false`

`$index` の `DoAction` または `DoInitAction` タグのアクションのバイトコードを返します
（`DoInitAction` では、スプライト ID を除きます）。アクションのタグでなければ
`false` を返します。

#### `disasmActionData(string $bytecode): bool`

`$bytecode`（たとえば `getActionData()` の結果）の逆アセンブルを、1 行に 1 つの
アクションで、PHP の出力に表示します。読み込んだムービーは使いません。常に
`true` を返します。

#### `setActionVariables(array $vars): bool`

`key => value` の組ごとに、配列の順で、`key = value` のバイトコードを、最初の
フレームの先頭に挿入します。そのフレームの最初の `DoAction` タグの中に入れ、
無ければ新しい `DoAction` タグを作ります。キーと値は文字列に変換されます。ムービーを
読み込んでいない場合や、キーまたは値が 65533 バイトより長い場合は、`false` を
返します。

#### `replaceActionStrings(array $map): bool`

ActionScript のバイトコードの中の、`$map` のキーと完全に等しい（バイトの比較で、
文字列全体だけ）すべての文字列を、対応する値に置き換えます。`Push`、
`ConstantPool`、`GetURL`（URL とターゲット）の文字列を扱います。対象は、最上位と
`DefineSprite` の中の、`DoAction` / `DoInitAction` タグ、`DefineButton` /
`DefineButton2` のアクション、`PlaceObject2` / `PlaceObject3` のクリップアクション
（`onClipEvent`）です。何も一致しなかったときも `true` を
返します。バイトコードが壊れている場合や、アクションが 65535 バイトを超えて
しまう場合は、`false` を返します。

### ムービークリップ

#### `replaceMovieClip(string $target, string $swf [, bool $unused_cid_purge]): bool`

インスタンス名が `$target` に完全に一致する `PlaceObject2` / `PlaceObject3` タグを探し、それが配置
している `DefineSprite` の中身を、`$swf`（完全な SWF ファイル）のメインタイムラインで
置き換えます。入れ子のクリップは、`"parent/child"` のようにパスで指定します。
各階層は、前のスプライトの中から探します。

- `$swf` の定義タグ（シェイプ、ビットマップ、フォント、テキスト、サウンド、ボタン、
  スプライト、`ExportAssets`、`DoInitAction` など）は、スプライトの前に挿入されます。
  キャラクター ID は、既存のものとぶつからないように振り直され、それを参照している
  側も追従します。
- 制御タグ（`ShowFrame`、`PlaceObject` / `2` / `3`、`RemoveObject` / `2`、`DoAction`、
  `StartSound`、`SoundStreamHead` / `Block`、`FrameLabel`、`End` など）は、スプライトの
  タイムラインになります。フレーム数は `ShowFrame` タグの数です。
- ムービー単位のタグ（`SetBackgroundColor`、`FileAttributes`、`Metadata`、`Protect`
  など）と、`$swf` のヘッダは捨てられます。`$swf` の `JPEGTables` は、その
  `DefineBitsJPEG` タグに組み込まれます。
- 古いキャラクターはファイルに残ります。`$unused_cid_purge` を `true` にすると、
  続けて `purgeUselessContents()` を実行します。

次の場合は、ムービーを変更せずに `false` を返します。`$target` が見つからない、
`$swf` を解析できない、`$swf` にスプライトへ入れられないタグがある（`DoABC`、
`SymbolClass`、`ImportAssets`、swfed が知らない種類のタグ、`DefineSprite` の中の
`DefineSprite`）、`$swf` のタグが、`$swf` が定義していないキャラクターを参照している。

### その他

#### `static getLastError(): ?string`

直前の `SWFEditor` のメソッド呼び出しの間に C のコードが出したメッセージを、
1 行に 1 つで返します。無ければ `null` を返します。すべてのメソッドは、開始時に
メッセージを消します。引数が間違っていて早く戻る場合も同じなので、メッセージが
以前の呼び出しのものになることはありません。`clone` も消します。コンストラクタと
`getLastError()` 自身は消しません。`swfed.error_output` には関係ありません。
メソッドは、メッセージ無しで `false` を返すことがあります（たとえば、説明できない
タグに対する `getTagDetail()`）。そのため、失敗のあとでも結果が `null` に
なることがあります。

#### `_destroy_and_exit(): bool`

デバッグ用の名残です。何もせず、`true` を返します。

## サンプル

`sample/` ディレクトリには、機能ごとに 1 つずつ、小さなコマンドラインのスクリプトが
あります。多くは SWF ファイルを受け取り、結果を標準出力に書き出します。たとえば:

```sh
php sample/swfreplacebitmapdata.php input.swf 1 new.png > output.swf
```

| スクリプト | 内容 |
|---|---|
| `swfdump.php` | ヘッダとすべてのタグを表示する（`swfInfo()`） |
| `swfgetheaderinfo.php`、`swfsetheaderinfo.php` | ヘッダのフィールドを読む / 変更する |
| `swfcompress.php` | SWF を圧縮する（`-u` で展開） |
| `swfreplacebitmapdata.php` | ビットマップを JPEG、PNG、GIF のファイルで置換する（+ 任意のアルファのファイル） |
| `swfreplacebitmapdata2.php` | 同じことを、`without_converting` / `rgb15` オプションで行う |
| `swfreplacejpegdata.php`、`swfreplacepngdata.php`、`swfreplacegifdata.php` | 形式ごとの置換 |
| `swfgetjpegdata.php`、`swfgetjpegalphadata.php`、`swfgetpngdata.php` | ビットマップを取り出す |
| `swfgeteditstring.php`、`swfreplaceeditstring.php` | テキストフィールドの内容を読む / 置換する |
| `swfsetactionvariables.php`、`swfreplaceactionstrings.php` | 変数を注入する / アクションの文字列を置換する |
| `swfreplacemovieclip.php` | ムービークリップを別の SWF で置換する |
| `swfgetshapedata.php`、`swfreplaceshapedata.php` | シェイプを取り出す / 置換する |
| `swfremovetag.php`、`swfpurgeuselesscontents.php`、`swfrebuild.php` | タグの整理 |

`src/tests/*.phpt` のテストも、短い使用例になっています。

## デモページ（`www/`）

`www/` の PHP ページは、SWF をアップロードし、その画像とシェイプを一覧し、画像を
置換します。PHP の組み込みサーバーで試せます（拡張モジュールを読み込む必要が
あります）。

```
cd www
php -d extension=swfed -S 127.0.0.1:8000
```

ページは SWF ファイルを表示します。`<object>` / `<embed>` で表示するには Flash
Player が必要ですが、今のブラウザはそれを実行できません。そのため、JavaScript で
書かれたプレーヤー [Flapp](https://github.com/yoya/Flapp)（`<flapp-player>` 要素）で
SWF を再生します。`flapp.js` は、コミットに固定した jsDelivr から読み込みます。
SWF はステージの大きさで表示され、クリックしなくても再生が始まります。

| 環境変数 | |
|---|---|
| `SWFED_PLAYER=none` | 何も読み込まず、`<object>` / `<embed>` をそのまま使う（まだ Flash Player があるブラウザ向け） |
| `SWFED_PLAYER_URL` | 自前の `flapp.js` のコピーの URL（`http(s)://...` またはパス）。jsDelivr に届かないネットワーク向け |

アップロードされたファイルは、システムの一時ディレクトリの下の `swfed-<uid>/` に
保存され（`www/define.php` の `SWFED_TMP_DIR` を参照）、1 時間後に削除されます。

## リンク

- リポジトリ: https://github.com/yoya/swfed
- Issues: https://github.com/yoya/swfed/issues

過去のリンク（このプロジェクトはかつて OSDN にありました。これらのページはもう
見られないかもしれません）:

- OSDN プロジェクト: https://osdn.jp/projects/swfed/
- デモ: http://app.awm.jp/swfed/
- 記事:
  - JPEG の置換: http://diary.awm.jp/~yoya/?20080331#200803311
  - アルファの置換: http://diary.awm.jp/~yoya/?20080514#200805141
  - ビットマップの調整: http://diary.awm.jp/~yoya/?20101027#201010270
  - アクションのバイトコード: http://diary.awm.jp/~yoya/?20101130#201011300
  - シェイプの置換: http://diary.awm.jp/~yoya/?20110203#201102030
  - ムービークリップの置換: http://diary.awm.jp/~yoya/?20110319#201103190
  - ビットマップの置換: http://diary.awm.jp/~yoya/?20110623#201106231

## ライセンス

`src/php_swfed.c` と `src/php_swfed.h` は、ヘッダに書かれているとおり、
PHP License 3.01 です。ほかのソースファイルには、作者の行だけがあります。

## 作者

Yoshihiro Yamazaki
