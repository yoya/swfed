# swfed - SWF Editor for PHP

swfed is a PHP extension for editing Adobe Flash (SWF) files.
It provides one class, `SWFEditor`, that loads an SWF from a string,
inspects and edits it at the tag level, and writes it back to a string.

What it can do:

- read and change the SWF header (version, compression, frame size)
- list, extract, replace and remove tags
- replace bitmaps with JPEG, PNG or GIF data, and extract them as JPEG / PNG
- edit the initial text of text fields (DefineEditText)
- replace strings in ActionScript 1/2 bytecode and inject variables
- replace the contents of a movie clip with another SWF
- resize or rescale shapes, and remove unused shapes and bitmaps

Tags that swfed does not understand (for example ActionScript 3 `DoABC`) are
copied through unchanged.

## Requirements

- PHP 7.2 or later, with `phpize` (usually in the `php-dev` / `php-devel` package).
  CI builds and tests PHP 7.2, 8.0 and 8.3.
- zlib, with headers. `configure` stops if it is not found.
- libpng, giflib and liblzma (xz), with headers. They are optional: without one of
  them, `configure` only warns and builds without that feature (PNG, GIF, or
  reading and writing LZMA-compressed `ZWS` files).
  - Debian / Ubuntu: `apt-get install zlib1g-dev libpng-dev libgif-dev liblzma-dev`
  - Homebrew: `brew install zlib libpng giflib xz`
- A Unix-like OS (Linux, macOS, *BSD). Windows builds are not covered by CI.

## Build and install

```sh
git clone https://github.com/yoya/swfed.git
cd swfed/src
phpize
./configure
make
make test
sudo make install
```

Then enable the extension in `php.ini` (or a file in your `conf.d` directory):

```ini
extension=swfed
```

Check it with `php --ri swfed`; it prints the version and whether PNG, GIF and
LZMA (ZWS) support were compiled in.

`configure` first uses a library as it is when the compiler finds it without any
option (system directories, the macOS SDK). Otherwise it looks for it with
`pkg-config`, then under `/usr/local`, `/usr`, `/opt/homebrew` and `/opt/local`.
If a library is installed elsewhere, pass its install prefix (the directory that
contains `include/` and `lib/`):

| option | library |
|---|---|
| `--with-zlib-dir=DIR` | zlib (`DIR/include/zlib.h`) |
| `--with-png-dir=DIR` | libpng (`DIR/include/png.h` or `DIR/include/libpng/png.h`) |
| `--with-gif-dir=DIR` | giflib (`DIR/include/gif_lib.h` or `DIR/include/giflib/gif_lib.h`) |
| `--with-lzma-dir=DIR` | liblzma (`DIR/include/lzma.h`) |

`--without-png-dir`, `--without-gif-dir` and `--without-lzma-dir` build without
that library.

For example, with Homebrew on Apple Silicon, when `./configure` alone does not
find them:

```sh
./configure --with-zlib-dir=$(brew --prefix zlib) --with-png-dir=$(brew --prefix) \
            --with-gif-dir=$(brew --prefix) --with-lzma-dir=$(brew --prefix xz)
```

See also [doc/install.txt](doc/install.txt).

## Quick example

```php
<?php
$swf = new SWFEditor();
if (! $swf->input(file_get_contents('input.swf'))) {
    exit("input failed\n");
}
// replace the bitmap whose character ID is 1 (JPEG, PNG or GIF data)
if (! $swf->replaceBitmapData(1, file_get_contents('new.jpg'))) {
    exit("replaceBitmapData failed\n");
}
file_put_contents('output.swf', $swf->output());
```

To find the IDs, run `php sample/swfdump.php input.swf`, or use
`getTagList()` and `getTagDetail()`.

## Basics

### Tag index and character ID

Methods address tags in one of two ways:

- **tag index**: the 0-based position of a tag in the top-level tag list, in the
  order returned by `getTagList()`. Tags inside a `DefineSprite` are not counted.
  Indexes shift when tags are removed or inserted.
  Used by `getTagDetail()`, `getTagData()`, `replaceTagData()`, `removeTag()` and
  `getActionData()`.
- **character ID** (CID): the ID of a `Define*` tag (`image_id`, `shape_id`,
  `sound_id`, ... in `getTagDetail()`). Used by all other methods that take an ID.
  Only top-level tags are searched.

The PHP stub names the ID parameter `$index` for several bitmap and sound methods
(`getJpegData()`, `replacePNGData()`, `getSoundData()`, ...). It is a character
ID, not a tag index. Keep this in mind if you call them with named arguments.

Sizes and positions (`getHeaderInfo()`, `getBitmapSize()`, the `trans_x` /
`trans_y` arguments) are in pixels. SWF stores twips (1 pixel = 20 twips);
`getHeaderInfo()` truncates to whole pixels.

### Strings and character encoding

swfed never converts character encodings. Every string you pass or get back
(edit text, action strings and variables, instance names, `getTagDetail()`
names) is the raw byte sequence stored in the SWF.

The SWF format stores strings as UTF-8 from SWF version 6 on. SWF 5 and earlier
use the author's local encoding, for example Shift_JIS for Japanese (common in
Flash Lite content). Check the version and convert yourself:

```php
$info = $swf->getHeaderInfo();
$enc  = ($info['version'] >= 6) ? 'UTF-8' : 'SJIS-win';

echo mb_convert_encoding($swf->getEditString('title'), 'UTF-8', $enc);
$swf->replaceEditString('title', mb_convert_encoding('new title', $enc, 'UTF-8'));
```

`www/index.php` uses the same rule (`SJIS-win` for versions below 6) when it
displays tag details.
Strings in SWF are NUL-terminated, so a NUL byte in a new string cuts it short.

### Errors

- Most methods return `false` on failure. SWF-level errors do not throw.
- Most diagnostics are written by the C code directly to the process's stderr.
  They are not PHP warnings, so `error_reporting()`, `@` and error handlers
  do not see them.
- A few failures raise a PHP `E_WARNING` and return `false`; they are listed
  with the methods below. Passing `null` as the first argument of
  `replaceBitmapData()`, `applyShapeMatrixFactor()`, `applyShapeRectFactor()`,
  `getSoundData()`, `replaceMLDData()`, `getEditString()`,
  `replaceEditString()` or `replaceMovieClip()` gives the warning
  "1st arg must be not NULL".
- Wrong argument types or counts throw `TypeError` / `ArgumentCountError` on
  PHP 8. On PHP 7 they raise a warning and the method returns `false` or `null`.
- `swfInfo()`, `printTagData()` and `disasmActionData()` print with C `printf()`.
  The output goes straight to the process's stdout and bypasses PHP output
  buffering (`ob_start()` does not capture it). Use them from the CLI.
- If a `replace*Data()` call fails, the target bitmap tag may already have been
  changed (for example its tag type switched) and can be left broken; a tag left
  with no data at all is silently left out of `output()`. Keep the original SWF
  and call `input()` again after a failed replace.

### The SWFEditor object

- `new SWFEditor()` creates an empty editor. Calling `__construct()` again on the
  same object throws `Error`.
- Subclasses work even if their constructor does not call `parent::__construct()`.
- `clone` makes an independent copy, including the loaded SWF and settings.
- `serialize()` throws an `Exception`; serialize the result of `output()` instead.
- Methods can be called before `input()`; they then work on an empty movie
  (for example `output()` returns `false`).

## API reference

Signatures show what the implementation accepts. Optional arguments are in
brackets.

### Input, output and header

#### `input(string $data): bool`

Parses an SWF file and replaces whatever was loaded before. Uncompressed (`FWS`),
zlib-compressed (`CWS`) and LZMA-compressed (`ZWS`) files are supported. `ZWS`
needs a build with liblzma; without it a `ZWS` file makes `input()` return `false`
(see `php --ri swfed`). Returns `false` if the header cannot be read, the signature
is not `FWS` / `CWS` / `ZWS`, or decompression fails (for a `ZWS`: damaged or
truncated LZMA data, or a size that does not match the header's file length); the
previously loaded movie is then kept.
Truncated tag data is not an error: parsing stops at the first incomplete tag and
`true` is returned.

#### `output(): string|false`

Serializes the movie. The file format follows the header: `FWS`, `CWS` if the
`compress` flag is set, or `ZWS` if `lzma` is also set (a `ZWS` that was read
stays `ZWS`). Compression uses the level from `setCompressLevel()`. The file
length field is recomputed. Returns `false` if nothing has been loaded or
compression fails, and for `ZWS` on a build without liblzma.

#### `getHeaderInfo(): array`

Returns:

| key | type | meaning |
|---|---|---|
| `compress` | bool | `true` for `CWS` and `ZWS`, `false` for `FWS` |
| `lzma` | bool | `true` for `ZWS` (LZMA), `false` for `FWS` and `CWS` |
| `version` | int | SWF version |
| `x_min`, `y_min`, `x_max`, `y_max` | int | frame rectangle in pixels |

Frame rate and frame count are not returned. Before `input()` all values are
0 / `false`.

#### `setHeaderInfo(array $header): bool`

Sets any of the `getHeaderInfo()` keys present in `$header`; other keys are
ignored. The file format is chosen from `compress` and `lzma`:

| given | result |
|---|---|
| `compress => false` | `FWS` (even if `lzma => true` is also given) |
| `lzma => true` | `ZWS` |
| `compress => true` (no `lzma`) | `CWS`; a movie that is already `ZWS` stays `ZWS` |
| `lzma => false` | `ZWS` becomes `CWS`; `CWS` stays `CWS`; `FWS` stays `FWS`, unless `compress => true` is also given (then `CWS`) |

So the array from `getHeaderInfo()` can be changed in one key and passed back.
Returns `true`. Returns `false` with an `E_WARNING`, and changes nothing, if
`version` is not between 0 and 255, a frame rectangle value is out of range, or
`lzma => true` is given on a build without liblzma. A non-array argument is an
error (see "Errors").
swfed does not adjust the version when you set `compress`; compressed files need
version 6 or later (`CWS`) or 13 or later (`ZWS`). `sample/swfcompress.php` raises
it to 6.

#### `setCompressLevel(int $level): bool`

zlib level used by `output()` for `CWS` files (for `ZWS` files, the LZMA preset:
`0` to `9`, and `-1` means the LZMA default) and for the zlib data inside
`DefineBitsJPEG3`/`DefineBitsJPEG4` (alpha) and `DefineBitsLossless`/`DefineBitsLossless2` tags
that swfed rebuilds. `-1` (zlib default, the initial value) to `9`. Out of range:
`E_WARNING` and `false`.

#### `swfInfo(): bool`

Prints a readable dump of the header and every tag to stdout (see Errors above).
Always returns `true`.

#### `rebuild(): bool`

Parses every tag that swfed understands, so that `output()` writes it from the
parsed data instead of copying the original bytes. Returns `false` if a tag
cannot be parsed (it stops at that tag). The output is usually not byte-identical
to the input.

#### `purgeUselessContents(): bool`

Removes shape (`DefineShape`, `DefineShape2`, `DefineShape3`, `DefineMorphShape`)
and bitmap (`DefineBits*`) tags that are not used by any `PlaceObject` /
`PlaceObject2`, directly or through a shape, button or placed sprite. Other
character types (sprites, fonts, texts, sounds, ...) are kept. `ExportAssets`
is not taken into account, so shapes and bitmaps that are only used through a
linkage name (e.g. `BitmapData.loadBitmap()`) are removed too. Returns `true`.

### Tags

#### `getTagList(): array`

Returns one array per top-level tag, in file order:

| key | type | meaning |
|---|---|---|
| `code` | int | tag type |
| `tag` | int | same as `code` |
| `tagName` | string | tag name; missing for tag types swfed does not know |
| `length` | int | tag body length in bytes, without the tag header. `0` for tags edited since the last `input()` / `output()` |
| `detail` | bool | `true` if swfed has a parser for this tag type; missing otherwise |

#### `getTagDetail(int $index): array|false|null`

Returns parsed information about the tag at `$index`. Keys by tag type:

| tag (code) | keys |
|---|---|
| DefineBitsJPEG (6), DefineBitsJPEG2 (21), DefineBitsJPEG3 (35), DefineBitsJPEG4 (90) | `image_id`, `jpeg_data_len`, `alpha_data_len` (only if there is alpha data) |
| DefineBitsLossless (20), DefineBitsLossless2 (36) | `image_id`, `format` (3 = 8-bit colormap, 4 = 15-bit RGB, 5 = 32-bit), `width`, `height`, `colormap_count` (format 3 only) |
| DefineSound (14) | `sound_id`, `format`, `rate`, `is_16bits`, `is_stereo`, `sound_samples_count`, `sound_data_len` (raw SWF field values) |
| DoAction (12), DoInitAction (59) | `action_list_count` (missing if empty), `action_sprite` (DoInitAction only) |
| DefineEditText (37) | `edit_id`, `variable_name` and `initial_text` (each only if not empty) |
| DefineSprite (39) | `sprite_id`, `frame_count` |
| DefineShape (2), DefineShape2 (22), DefineShape3 (32), DefineMorphShape (46) | `shape_id`, `fill_styles.count`, `line_styles.count`, `bitmap_ref` (list of referenced bitmap IDs; missing if none) |
| PlaceObject (4), PlaceObject2 (26) | `character_id` (PlaceObject2: only if set), `depth`, `name` (only if set) |

Returns `false` if `$index` is out of range or negative, the tag type has no
parser, or the tag cannot be parsed. Returns `null` for `JPEGTables` (8),
`DefineButton` (7) and `DefineButton2` (34): `getTagList()` reports `detail` for
them but no keys are implemented. `DefineShape4` (83) and `DefineMorphShape2` (84)
also report `detail` but return `false`.

#### `getTagData(int $index): string|false`

Returns the complete tag at `$index` (tag header and body) as `output()` would
write it. Returns `false` if `$index` is out of range. A negative `$index` is
treated as 0.

#### `replaceTagData(int $index, string $data): bool`

Replaces the tag at `$index` with `$data`, which must be one complete tag
(header and body, e.g. from `getTagData()`). Only the tag header is checked.
Returns `false` if `$index` is out of range or `$data` is not a complete tag.
A negative `$index` is treated as 0.

#### `getTagDataByCID(int $cid): string|false`

Returns the complete tag (header and body) that defines character `$cid`, or
`false`.

#### `replaceTagDataByCID(int $cid, string $data): bool`

Replaces the tag that defines `$cid` with `$data` (one complete tag). The
character ID inside `$data` is rewritten to `$cid`; the tag type may change.
Returns `false` if `$cid` is not found or `$data` is not a complete tag.

#### `getTagContentsByCID(int $cid): string|false`

Returns the body of the tag that defines `$cid`, without the tag header and
without the leading 2-byte character ID. Returns `false` if not found.

#### `replaceTagContentsByCID(int $cid, string $data): bool`

Replaces the body after the character ID (the counterpart of
`getTagContentsByCID()`). The tag type and ID are kept; `$data` is not checked.
Returns `false` if `$cid` is not found.

#### `removeTag(int $index [, ?int $index_in_sprite = null]): bool`

Removes the top-level tag at `$index`. With `$index_in_sprite`, removes the tag at
that position inside the `DefineSprite` at `$index` instead; `null` or a negative
value means "not given". Returns `false` if an index is out of range or the tag at
`$index` is not a `DefineSprite` when `$index_in_sprite` is given. A negative
`$index` is treated as 0. (The stub calls the second parameter `$index_sprint`.)

#### `printTagData(string $data): bool`

Prints one complete tag (e.g. from `getTagData()`) to stdout in the `swfInfo()`
format. Returns `false` if `$data` is not a complete tag.

#### `isShapeTagData(string $data): bool`

Returns `true` if `$data` is a complete tag whose type is `DefineShape`,
`DefineShape2`, `DefineShape3` or `DefineMorphShape`. Only the tag header is
checked. Does not use the loaded movie.

#### `isBitmapTagData(string $data): bool`

Same as `isShapeTagData()` for `DefineBitsJPEG`, `DefineBitsJPEG2`,
`DefineBitsJPEG3`, `DefineBitsJPEG4`, `DefineBitsLossless` and `DefineBitsLossless2`.

### Shapes

These methods handle `DefineShape`, `DefineShape2`, `DefineShape3` and (for get /
replace) `DefineMorphShape`. `DefineShape4` is not supported.

#### `getShapeData(int $cid): string|false`

Returns the complete shape tag `$cid` (header and body). Returns `false` if `$cid`
is not found or is not a shape.

#### `replaceShapeData(int $cid, string $data): bool`

Replaces shape `$cid` with `$data`, a complete shape tag (e.g. from
`getShapeData()`, possibly from another SWF). The shape ID inside `$data` is
rewritten to `$cid` and the tag type is taken from `$data`. Bitmap IDs referenced
by the new shape are used as they are. If `$data` is not a shape tag, it is read
as a shape body without the ID (the format of swfed 0.37 and earlier). Returns
`false` if `$cid` is not a shape or `$data` cannot be parsed.

#### `applyShapeMatrixFactor(int $shape_id, float $scale_x, float $scale_y, float $rotate_rad, int $trans_x, int $trans_y): bool`

Transforms the fill matrices of every bitmap and gradient fill in the shape:
scales them, rotates them by `$rotate_rad` radians, then moves them by
(`$trans_x`, `$trans_y`) pixels. The outline is not changed. Returns `false` if no
`DefineShape`/`2`/`3` with that ID exists (`DefineMorphShape` is not supported).

#### `applyShapeRectFactor(int $shape_id, float $scale_x, float $scale_y, int $trans_x, int $trans_y): bool`

Scales the shape outline and bounds around their top-left corner, then moves
them by (`$trans_x`, `$trans_y`) pixels. Fill matrices are not changed. Returns
`false` like `applyShapeMatrixFactor()`.

#### `setShapeAdjustMode(int $mode): bool`

Sets how shapes that use a bitmap are adjusted when that bitmap is replaced by
`replaceJpegData()`, `replacePNGData()`, `replaceGIFData()` or
`replaceBitmapData()`. The mode is a bit mask, kept until changed, and may be
set before `input()`. Always returns `true`; the value is not checked.

| constant | value | effect on each shape that uses the replaced bitmap |
|---|---|---|
| `SWFEditor::SHAPE_BITMAP_NONE` | 0 | nothing (default) |
| `SWFEditor::SHAPE_BITMAP_MATRIX_RESCALE` | 1 | the bitmap fill matrix is scaled by old size / new size, so the new image fills the same area as the old one |
| `SWFEditor::SHAPE_BITMAP_RECT_RESIZE` | 2 | the shape outline and bounds are scaled by new size / old size, so the shape takes the new image's size |
| `SWFEditor::SHAPE_BITMAP_TYPE_TILLED` | 4 | clipped bitmap fills are changed to tiled (repeating) fills |

Combine values with `|`. The samples use `SHAPE_BITMAP_RECT_RESIZE`.

#### `getShapeIdListByBitmapRef(int $bitmap_id): array`

Returns the IDs of the shapes whose fill styles use bitmap `$bitmap_id`, or an
empty array.

### Bitmaps

A replace method accepts any bitmap tag (JPEG or lossless) as the target and
changes its tag type to match the new data. See "Errors" above for what a failed
replace leaves behind.

#### `getBitmapSize(int $bitmap_id): array|false`

Returns `['width' => int, 'height' => int]` in pixels, or `false` if `$bitmap_id`
is not a bitmap or the JPEG size cannot be read.

#### `getJpegData(int $image_id): string|false`

Returns the image of a `DefineBitsJPEG`/`2`/`3`/`4` tag as a normal JPEG file (for
`DefineBitsJPEG` the shared `JPEGTables` are merged in). Returns `false` if there
is no JPEG tag with that ID.

#### `getJpegAlpha(int $image_id): string|false`

Returns the alpha channel of a `DefineBitsJPEG3` or `DefineBitsJPEG4` tag,
uncompressed: one byte per pixel, width × height bytes. Returns `false` if the
tag is neither of them or has no alpha data.

#### `replaceJpegData(int $image_id, string $jpeg [, string $alpha]): bool`

Replaces bitmap `$image_id` with JPEG data. Without `$alpha` (or with `''`) the
tag becomes `DefineBitsJPEG2`; with `$alpha` it becomes `DefineBitsJPEG3`, or
stays `DefineBitsJPEG4` (keeping its `DeblockParam`) if it already was one.
`$alpha` is uncompressed alpha data in the `getJpegAlpha()` format; swfed
compresses it. Neither the JPEG data nor the alpha length is checked, so this
returns `true` for non-JPEG data. Returns `false` if there is no bitmap with that
ID. Passing `null` as `$alpha` works but raises `E_DEPRECATED` on PHP 8.1+; omit
it instead.

#### `getPNGData(int $image_id): string|false`

Returns a `DefineBitsLossless`/`2` tag as a PNG file. Returns `false` for other
tags and for 15-bit (format 4) bitmaps.

#### `replacePNGData(int $image_id, string $png [, array $options]): bool`

Replaces bitmap `$image_id` with a PNG image:

| PNG | result |
|---|---|
| palette | format 3; `DefineBitsLossless2` if it has transparency (tRNS), else `DefineBitsLossless` |
| RGB | `DefineBitsLossless`, format 5 (format 4 with `rgb15`) |
| RGB + alpha | `DefineBitsLossless2`, format 5 |

Grayscale PNGs and PNGs with more than 8 bits per channel are not supported.
Option keys:

- `rgb15` (bool): store an RGB PNG as 15-bit color (format 4). Ignored, with a
  message on stderr, for other PNG types.

If given, `$options` must be an array; `null` throws `TypeError`.
Returns `false` if there is no bitmap with that ID or the PNG cannot be converted.

#### `replaceGIFData(int $image_id, string $gif): bool`

Replaces bitmap `$image_id` with the first image of a GIF file
(`DefineBitsLossless`, format 3; `DefineBitsLossless2` if the GIF has a
transparent color). Returns `false` if there is no bitmap with that ID or the GIF
cannot be read.

#### `replaceBitmapData(int|array $target, string $data [, string $alpha [, bool|array $options]]): bool`

Detects whether `$data` is JPEG, PNG or GIF from its signature and replaces the
bitmap like `replaceJpegData()`, `replacePNGData()` or `replaceGIFData()`.

`$target` is a character ID (non-array values are converted to int) or an array
of conditions. With conditions, the first bitmap tag in file order that matches
all given keys is replaced:

| key | match |
|---|---|
| `width`, `height` | exact size in pixels |
| `red`, `green`, `blue` | color of the first pixel, within ±10. Only lossless bitmaps can match these. |

`$alpha` is used for JPEG data only. To pass `$options` without alpha data,
pass `''` as `$alpha` (`null` raises `E_DEPRECATED` on PHP 8.1+).

`$options` is either a bool (same as `without_converting`) or an array with the
keys:

- `without_converting` (bool): store the file bytes unchanged in a
  `DefineBitsJPEG2` tag (JPEG, PNG or GIF; PNG and GIF in this tag are a
  SWF 8 feature). `$alpha` and `rgb15` are ignored.
- `rgb15` (bool): as for `replacePNGData()`.

Returns `false` on failure. These cases also raise `E_WARNING`: unknown image
format ("Unknown Bitmap Format"), no bitmap matching the conditions, and `null`
as `$target`.

#### `convertBitmapDataToJpegTag(): bool`

Converts every top-level `DefineBitsLossless`/`2` tag into a `DefineBitsJPEG2`
tag that holds the image as PNG (lossless; needs SWF 8 / Flash Player 8 to
display). Bitmaps that cannot be converted are left unchanged. Always returns
`true`.

### Sounds

#### `getSoundData(int $sound_id): string|false`

Returns the sound data of the `DefineSound` tag `$sound_id`: the bytes after the
tag's format fields, in the tag's own format (for MP3 this starts with the 2-byte
SeekSamples field). Use `getTagDetail()` for the format and rate. Returns `false`
if not found.

#### `replaceMLDData(int $sound_id, string $mld): bool`

Replaces the data of `DefineSound` `$sound_id` with MLD (i-mode melody) data and
sets the sound format to 15 (device sound, used by Flash Lite), rate 0, 8-bit,
mono, sample count 0. The data is not checked. Returns `false` if not found.

### Text fields

#### `getEditString(string $name): string|null|false`

Returns the initial text of the first `DefineEditText` whose variable name is
exactly `$name`. If the variable name does not match, `$name` is also tried as a
decimal character ID (e.g. `"2"`). Returns `null` if the matching field has no
initial text (or no movie is loaded), and `false` if no field matches. The text
is returned as stored (see "Strings and character encoding").

#### `replaceEditString(string $name, string $text): bool`

Sets the initial text of every `DefineEditText` that matches `$name` (same rule
as `getEditString()`). An empty `$text` removes the initial text. Returns `false`
if no field matches.

### Actions (ActionScript 1/2)

#### `getActionData(int $index): string|false`

Returns the action bytecode of the `DoAction` or `DoInitAction` tag at
`$index` (for `DoInitAction`, without the sprite ID). Returns `false` if the tag
is not an action tag.

#### `disasmActionData(string $bytecode): bool`

Prints a disassembly of `$bytecode` (e.g. from `getActionData()`) to stdout, one
action per line. Does not use the loaded movie. Always returns `true`.

#### `setActionVariables(array $vars): bool`

For each `key => value` pair, in array order, inserts the bytecode for
`key = value` at the start of the first frame: into the first `DoAction` tag of
that frame, or into a new `DoAction` tag if the frame has none. Keys and values
are converted to strings. Returns `false` if no movie is loaded or a key or
value is longer than 65533 bytes.

#### `replaceActionStrings(array $map): bool`

Replaces every string in ActionScript bytecode that is exactly equal to a key of
`$map` (byte comparison, whole strings only) with the corresponding value.
Strings in `Push`, `ConstantPool` and `GetURL` (URL and target) are handled, in
`DoAction` / `DoInitAction` tags at the top level and inside `DefineSprite`.
Button actions and clip event actions are not changed. Returns `true` also when
nothing matched; `false` on broken bytecode or if an action would grow beyond
65535 bytes.

### Movie clips

#### `replaceMovieClip(string $target, string $swf [, bool $unused_cid_purge]): bool`

Finds the `PlaceObject2` tag whose instance name is exactly `$target` and replaces
the contents of the `DefineSprite` it places with the main timeline of `$swf`
(a complete SWF file). Nested clips are given as a path, `"parent/child"`; each
level is looked up inside the previous sprite.

- Character tags of `$swf` (shapes, bitmaps, fonts, texts, sounds, buttons,
  sprites, ...) are inserted before the sprite, with their character IDs
  renumbered so they do not collide with existing ones.
- Control tags (`ShowFrame`, `PlaceObject`/`2`, `RemoveObject`/`2`, `DoAction`,
  `DoInitAction`, `StartSound`, `FrameLabel`, `End`) become the sprite's
  timeline; the frame count is the number of `ShowFrame` tags.
- Other tags (`SetBackgroundColor`, `FileAttributes`, `ExportAssets`,
  `JPEGTables`, `CSMTextSettings`, `FreeCharacter` and tag types swfed does not
  know) and the header of `$swf` are dropped.
- The old characters stay in the file; call `purgeUselessContents()` to remove
  unused shapes and bitmaps.

`$unused_cid_purge` is accepted but ignored (passing `null` raises `E_DEPRECATED`
on PHP 8.1+). Returns `false` if `$target` is not found or `$swf` cannot be
parsed.

### Other

#### `_destroy_and_exit(): bool`

Debug leftover. Does nothing and returns `true`.

## Samples

The `sample/` directory has small command line scripts, one per feature. Most
take an SWF file and write the result to stdout, e.g.

```sh
php sample/swfreplacebitmapdata.php input.swf 1 new.png > output.swf
```

| script | what it does |
|---|---|
| `swfdump.php` | print the header and all tags (`swfInfo()`) |
| `swfgetheaderinfo.php`, `swfsetheaderinfo.php` | read / change header fields |
| `swfcompress.php` | compress an SWF (`-u` to uncompress) |
| `swfreplacebitmapdata.php` | replace a bitmap with a JPEG, PNG or GIF file (+ optional alpha file) |
| `swfreplacebitmapdata2.php` | the same with `without_converting` / `rgb15` options |
| `swfreplacejpegdata.php`, `swfreplacepngdata.php`, `swfreplacegifdata.php` | format-specific replacement |
| `swfgetjpegdata.php`, `swfgetjpegalphadata.php`, `swfgetpngdata.php` | extract a bitmap |
| `swfgeteditstring.php`, `swfreplaceeditstring.php` | read / replace text field contents |
| `swfsetactionvariables.php`, `swfreplaceactionstrings.php` | inject variables / replace action strings |
| `swfreplacemovieclip.php` | replace a movie clip with another SWF |
| `swfgetshapedata.php`, `swfreplaceshapedata.php` | extract / replace a shape |
| `swfremovetag.php`, `swfpurgeuselesscontents.php`, `swfrebuild.php` | tag maintenance |

The tests in `src/tests/*.phpt` are also short usage examples.

## Links

- Repository: https://github.com/yoya/swfed
- Issues: https://github.com/yoya/swfed/issues

Historical links (the project used to be hosted on OSDN; these pages may no
longer be available, and the blog articles are in Japanese):

- OSDN project: https://osdn.jp/projects/swfed/
- Demo: http://app.awm.jp/swfed/
- Articles: replace JPEG http://diary.awm.jp/~yoya/?20080331#200803311,
  replace alpha http://diary.awm.jp/~yoya/?20080514#200805141,
  adjust bitmap http://diary.awm.jp/~yoya/?20101027#201010270,
  action bytecode http://diary.awm.jp/~yoya/?20101130#201011300,
  replace shape http://diary.awm.jp/~yoya/?20110203#201102030,
  replace movie clip http://diary.awm.jp/~yoya/?20110319#201103190,
  replace bitmap http://diary.awm.jp/~yoya/?20110623#201106231

## License

`src/php_swfed.c` and `src/php_swfed.h` are under the PHP License 3.01, as stated
in their headers. The other source files carry only an author line.

## Author

Yoshihiro Yamazaki
