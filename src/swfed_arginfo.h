
ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

// IS_XXX defined https://github.com/php/php-src/blob/master/Zend/zend_types.h#L521

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_input, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, swf_object, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_output, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_swfinfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed__destroy_and_exit, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getHeaderInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_setHeaderInfo, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, header_info, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getTagList, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getTagDetail, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getTagData, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceTagData, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, seq_number, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, swf_object, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getTagDataByCID, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, seq_number, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceTagDataByCID, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, seq_number, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, swf_object, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getTagContentsByCID, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, seq_number, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceTagContentsByCID, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, seq_number, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, swf_object, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_removeTag, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, tag_seqno, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, tag_seqno_in_sprite, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_printTagData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getShapeData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, cid, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceShapeData, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, cid, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, swf_object, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_setShapeAdjustMode, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getShapeIdListByBitmapRef, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, bitmap_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getBitmapSize, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, bitmap_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getJpegData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getJpegAlpha, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceJpegData, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, alpha_data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getPNGData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replacePNGData, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, opts, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceGIFData, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, image_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceBitmapData, 0, 0, 4)
// php8
#if ZEND_MODULE_API_NO >= 20200930
ZEND_ARG_TYPE_INFO(0, arg1, IS_MIXED, 0)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, opts, IS_STRING, 1)
ZEND_ARG_TYPE_INFO(0, arg4, IS_MIXED, 1)
#else
ZEND_ARG_INFO(0, arg1)
ZEND_ARG_INFO(0, data)
ZEND_ARG_INFO(0, opts)
ZEND_ARG_INFO(0, arg4)
#endif
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_convertBitmapDataToJpegTag, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_applyShapeMatrixFactor, 0, 0, 6)
ZEND_ARG_TYPE_INFO(0, shape_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, scale_x, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, scale_y, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, rotate_rad, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, trans_x, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, trans_y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_applyShapeRectFactor, 0, 0, 5)
ZEND_ARG_TYPE_INFO(0, shape_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, scale_x, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, scale_y, IS_DOUBLE, 0)
ZEND_ARG_TYPE_INFO(0, trans_x, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, trans_y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getSoundData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, sound_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceMLDData, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, sound_id, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getEditString, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, var_name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceEditString, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, var_name, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, ini_text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_getActionData, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, tag_seqno, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_disasmActionData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_setActionVariables, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, arr, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceActionStrings, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, arr, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_replaceMovieClip, 0, 0, 3)
ZEND_ARG_TYPE_INFO(0, instance_name, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, swf_data, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, unused_cid_purge, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_rebuild, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_purgeUselessContents, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_setCompressLevel, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, compress_level, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_isShapeTagData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_swfed_isBitmapTagData, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(swfed, __construct);
ZEND_METHOD(swfed, input);
ZEND_METHOD(swfed, output);
ZEND_METHOD(swfed, swfInfo);
ZEND_METHOD(swfed, _destroy_and_exit);
ZEND_METHOD(swfed, getHeaderInfo);
ZEND_METHOD(swfed, setHeaderInfo);
ZEND_METHOD(swfed, getTagList);
ZEND_METHOD(swfed, getTagDetail);
ZEND_METHOD(swfed, getTagData);
ZEND_METHOD(swfed, replaceTagData);
ZEND_METHOD(swfed, getTagDataByCID);
ZEND_METHOD(swfed, replaceTagDataByCID);
ZEND_METHOD(swfed, getTagContentsByCID);
ZEND_METHOD(swfed, replaceTagContentsByCID);
ZEND_METHOD(swfed, removeTag);
ZEND_METHOD(swfed, printTagData);
ZEND_METHOD(swfed, getShapeData);
ZEND_METHOD(swfed, replaceShapeData);
ZEND_METHOD(swfed, setShapeAdjustMode);
ZEND_METHOD(swfed, getShapeIdListByBitmapRef);
ZEND_METHOD(swfed, getBitmapSize);
ZEND_METHOD(swfed, getJpegData);
ZEND_METHOD(swfed, getJpegAlpha);
ZEND_METHOD(swfed, replaceJpegData);
ZEND_METHOD(swfed, getPNGData);
ZEND_METHOD(swfed, replacePNGData);
ZEND_METHOD(swfed, replaceGIFData);
ZEND_METHOD(swfed, replaceBitmapData);
ZEND_METHOD(swfed, convertBitmapDataToJpegTag);
ZEND_METHOD(swfed, applyShapeMatrixFactor);
ZEND_METHOD(swfed, applyShapeRectFactor);
ZEND_METHOD(swfed, adjustShapeSizeToBitmap);
ZEND_METHOD(swfed, adjustShapeScaleToBitmap);
ZEND_METHOD(swfed, getSoundData);
ZEND_METHOD(swfed, replaceMLDData);
ZEND_METHOD(swfed, getEditString);
ZEND_METHOD(swfed, replaceEditString);
ZEND_METHOD(swfed, getActionData);
ZEND_METHOD(swfed, disasmActionData);
ZEND_METHOD(swfed, setActionVariables);
ZEND_METHOD(swfed, replaceActionStrings);
ZEND_METHOD(swfed, replaceMovieClip);
ZEND_METHOD(swfed, rebuild);
ZEND_METHOD(swfed, purgeUselessContents);
ZEND_METHOD(swfed, setCompressLevel);
ZEND_METHOD(swfed, isShapeTagData);
ZEND_METHOD(swfed, isBitmapTagData);

/* {{{ swfed_functions[]
 *
 * Every user visible function must have an entry in swfed_functions[].
 */
static const zend_function_entry swfed_functions[] = {
    PHP_ME(swfed,  __construct, arginfo_class_swfed___construct, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  input, arginfo_class_swfed_input, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  output, arginfo_class_swfed_output, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  swfInfo, arginfo_class_swfed_swfinfo, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  _destroy_and_exit, arginfo_class_swfed__destroy_and_exit, ZEND_ACC_PUBLIC) // for debug

    PHP_ME(swfed,  getHeaderInfo, arginfo_class_swfed_getHeaderInfo, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  setHeaderInfo, arginfo_class_swfed_setHeaderInfo, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getTagList, arginfo_class_swfed_getTagList, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getTagDetail, arginfo_class_swfed_getTagDetail, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getTagData, arginfo_class_swfed_getTagData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceTagData, arginfo_class_swfed_replaceTagData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getTagDataByCID, arginfo_class_swfed_getTagDataByCID, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceTagDataByCID, arginfo_class_swfed_replaceTagDataByCID, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getTagContentsByCID, arginfo_class_swfed_getTagContentsByCID, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceTagContentsByCID, arginfo_class_swfed_replaceTagContentsByCID, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  removeTag, arginfo_class_swfed_removeTag, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  printTagData, arginfo_class_swfed_printTagData, ZEND_ACC_PUBLIC)

    PHP_ME(swfed,  getShapeData, arginfo_class_swfed_getShapeData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceShapeData, arginfo_class_swfed_replaceShapeData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  setShapeAdjustMode, arginfo_class_swfed_setShapeAdjustMode, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getShapeIdListByBitmapRef, arginfo_class_swfed_getShapeIdListByBitmapRef, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getBitmapSize, arginfo_class_swfed_getBitmapSize, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getJpegData, arginfo_class_swfed_getJpegData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getJpegAlpha, arginfo_class_swfed_getJpegAlpha, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceJpegData, arginfo_class_swfed_replaceJpegData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getPNGData, arginfo_class_swfed_getPNGData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replacePNGData, arginfo_class_swfed_replacePNGData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceGIFData, arginfo_class_swfed_replaceGIFData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceBitmapData, arginfo_class_swfed_replaceBitmapData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  convertBitmapDataToJpegTag, arginfo_class_swfed_convertBitmapDataToJpegTag, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  applyShapeMatrixFactor, arginfo_class_swfed_applyShapeMatrixFactor, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  applyShapeRectFactor, arginfo_class_swfed_applyShapeRectFactor, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getSoundData, arginfo_class_swfed_getSoundData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceMLDData, arginfo_class_swfed_replaceMLDData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getEditString, arginfo_class_swfed_replaceEditString, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceEditString, arginfo_class_swfed_replaceEditString, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  getActionData, arginfo_class_swfed_getActionData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  disasmActionData, arginfo_class_swfed_disasmActionData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  setActionVariables, arginfo_class_swfed_setActionVariables, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceActionStrings, arginfo_class_swfed_replaceActionStrings, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  replaceMovieClip, arginfo_class_swfed_replaceMovieClip, ZEND_ACC_PUBLIC)

    PHP_ME(swfed,  setCompressLevel, arginfo_class_swfed_setCompressLevel, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  rebuild, arginfo_class_swfed_rebuild, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  purgeUselessContents, arginfo_class_swfed_purgeUselessContents, ZEND_ACC_PUBLIC)

    PHP_ME(swfed,  isShapeTagData, arginfo_class_swfed_isShapeTagData, ZEND_ACC_PUBLIC)
    PHP_ME(swfed,  isBitmapTagData, arginfo_class_swfed_isBitmapTagData, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};
/* }}} */
