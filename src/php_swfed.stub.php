<?php
/**
 * @generate-function-entries
 * @generate-legacy-arginfo
 * @generate-class-entries
 */
// php /usr/local/lib/php/build/gen_stub.php php_swfed.stub.php

class SWFEditor
{
    /**
     * @var int
     * @cvalue SWFED_SHAPE_BITMAP_NONE
     */
    const SHAPE_BITMAP_NONE = UNKNOWN;
    /**
     * @var int
     * @cvalue SWFED_SHAPE_BITMAP_MATRIX_RESCALE
     */
    const SHAPE_BITMAP_MATRIX_RESCALE = UNKNOWN;
    /**
     * @var int
     * @cvalue SWFED_SHAPE_BITMAP_RECT_RESIZE
     */
    const SHAPE_BITMAP_RECT_RESIZE = UNKNOWN;
    /**
     * @var int
     * @cvalue SWFED_SHAPE_BITMAP_TYPE_TILLED
     */
    const SHAPE_BITMAP_TYPE_TILLED = UNKNOWN;
    
    public function __construct() {}

    public function input(string $input): bool {}
    public function output(): false|string {}
    public function swfInfo(): true {}
    public function _destroy_and_exit(): bool {} // for debug

    public function getHeaderInfo(): array {}
    public function setHeaderInfo(array $header): bool{}
    public function getTagList(): false|array{}
    public function getTagDetail(int $index): false|array {}
    public function getTagData(int $index): false|array {}
    public function replaceTagData(int $index, string $data): bool {}
    public function getTagDataByCID(int $cid): false|string {}
    public function replaceTagDataByCID(int $cid, string $data): bool {}
    public function getTagContentsByCID(int $cid): false|string {}
    public function replaceTagContentsByCID(int $cid, string $data): bool {}
    public function removeTag(int $index, ?int $index_sprint): bool {}
    public function printTagData(string $data): bool {}
    public function getShapeData(int $cid): false|string{}
    public function replaceShapeData(int $cid, string $data): bool {}
    public function setShapeAdjustMode(int $mode): bool {}
    public function getShapeIdListByBitmapRef(int $bitmap_id): false|array {}
    public function getBitmapSize(int $bitmap_id): false|array {}
    public function getJpegData(int $index): false|string {}
    public function getJpegAlpha(int $index): false|string {}
    public function replaceJpegData(int $index, string $data, ?string $alpha_data): bool {}
    public function getPNGData(int $index): false|string {}
    public function replacePNGData(int $index, string $data, ?string $option): bool {}
    public function replaceGIFData(int $index, string $data): bool {}
    public function replaceBitmapData(mixed $arg1, string $data, ?string $data_alpha, ?mixed $arg4): bool {}
    public function convertBitmapDataToJpegTag(): bool {}
    public function applyShapeMatrixFactor(int $shape_id, float $scale_x, float $scale_y, float $rotate_rad, int $trans_x, int $trans_y): bool {}
    public function applyShapeRectFactor(int $shape_id, float $scale_x, float $scale_y, int $trans_x, int $trans_y): bool {}
    public function getSoundData(int $index): false|array {}
    public function replaceMLDData(int $index, string $data): bool {}
    public function getEditString(string $var_name): false|string {}
    public function replaceEditString(string $var_name, string $ini_text): bool {}
    public function getActionData(int $index): false|string {}
    public function disasmActionData(string $data): bool{}
    public function setActionVariables(array $arr): bool {}
    public function replaceActionStrings(array $arr): bool {}
    public function replaceMovieClip(string $instance_name, string $swf_data, ?bool $unused_cid_purge): bool {}
    public function setCompressLevel(int $compress_level): bool {}
    public function rebuild(): bool {}
    public function purgeUselessContents(): true {}
    public function isShapeTagData(string $data): bool {}
    public function isBitmapTagData(string $data): bool {}
}

