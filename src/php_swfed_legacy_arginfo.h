/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 86108e2c1250f91cab72e7c3a74ef96190b04245 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_input, 0, 0, 1)
	ZEND_ARG_INFO(0, input)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_output arginfo_class_SWFEditor___construct

#define arginfo_class_SWFEditor_swfInfo arginfo_class_SWFEditor___construct

#define arginfo_class_SWFEditor__destroy_and_exit arginfo_class_SWFEditor___construct

#define arginfo_class_SWFEditor_getHeaderInfo arginfo_class_SWFEditor___construct

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_setHeaderInfo, 0, 0, 1)
	ZEND_ARG_INFO(0, header)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getTagList arginfo_class_SWFEditor___construct

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_getTagDetail, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getTagData arginfo_class_SWFEditor_getTagDetail

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceTagData, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_getTagDataByCID, 0, 0, 1)
	ZEND_ARG_INFO(0, cid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceTagDataByCID, 0, 0, 2)
	ZEND_ARG_INFO(0, cid)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getTagContentsByCID arginfo_class_SWFEditor_getTagDataByCID

#define arginfo_class_SWFEditor_replaceTagContentsByCID arginfo_class_SWFEditor_replaceTagDataByCID

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_removeTag, 0, 0, 2)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, index_sprint)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_printTagData, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getShapeData arginfo_class_SWFEditor_getTagDataByCID

#define arginfo_class_SWFEditor_replaceShapeData arginfo_class_SWFEditor_replaceTagDataByCID

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_setShapeAdjustMode, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_getShapeIdListByBitmapRef, 0, 0, 1)
	ZEND_ARG_INFO(0, bitmap_id)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getBitmapSize arginfo_class_SWFEditor_getShapeIdListByBitmapRef

#define arginfo_class_SWFEditor_getJpegData arginfo_class_SWFEditor_getTagDetail

#define arginfo_class_SWFEditor_getJpegAlpha arginfo_class_SWFEditor_getTagDetail

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceJpegData, 0, 0, 3)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, alpha_data)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getPNGData arginfo_class_SWFEditor_getTagDetail

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replacePNGData, 0, 0, 3)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_replaceGIFData arginfo_class_SWFEditor_replaceTagData

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceBitmapData, 0, 0, 4)
	ZEND_ARG_INFO(0, arg1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, data_alpha)
	ZEND_ARG_INFO(0, arg4)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_convertBitmapDataToJpegTag arginfo_class_SWFEditor___construct

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_applyShapeMatrixFactor, 0, 0, 6)
	ZEND_ARG_INFO(0, shape_id)
	ZEND_ARG_INFO(0, scale_x)
	ZEND_ARG_INFO(0, scale_y)
	ZEND_ARG_INFO(0, rotate_rad)
	ZEND_ARG_INFO(0, trans_x)
	ZEND_ARG_INFO(0, trans_y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_applyShapeRectFactor, 0, 0, 5)
	ZEND_ARG_INFO(0, shape_id)
	ZEND_ARG_INFO(0, scale_x)
	ZEND_ARG_INFO(0, scale_y)
	ZEND_ARG_INFO(0, trans_x)
	ZEND_ARG_INFO(0, trans_y)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getSoundData arginfo_class_SWFEditor_getTagDetail

#define arginfo_class_SWFEditor_replaceMLDData arginfo_class_SWFEditor_replaceTagData

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_getEditString, 0, 0, 1)
	ZEND_ARG_INFO(0, var_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceEditString, 0, 0, 2)
	ZEND_ARG_INFO(0, var_name)
	ZEND_ARG_INFO(0, ini_text)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_getActionData arginfo_class_SWFEditor_getTagDetail

#define arginfo_class_SWFEditor_disasmActionData arginfo_class_SWFEditor_printTagData

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_setActionVariables, 0, 0, 1)
	ZEND_ARG_INFO(0, arr)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_replaceActionStrings arginfo_class_SWFEditor_setActionVariables

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_replaceMovieClip, 0, 0, 3)
	ZEND_ARG_INFO(0, instance_name)
	ZEND_ARG_INFO(0, swf_data)
	ZEND_ARG_INFO(0, unused_cid_purge)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_SWFEditor_setCompressLevel, 0, 0, 1)
	ZEND_ARG_INFO(0, compress_level)
ZEND_END_ARG_INFO()

#define arginfo_class_SWFEditor_rebuild arginfo_class_SWFEditor___construct

#define arginfo_class_SWFEditor_purgeUselessContents arginfo_class_SWFEditor___construct

#define arginfo_class_SWFEditor_isShapeTagData arginfo_class_SWFEditor_printTagData

#define arginfo_class_SWFEditor_isBitmapTagData arginfo_class_SWFEditor_printTagData


ZEND_METHOD(SWFEditor, __construct);
ZEND_METHOD(SWFEditor, input);
ZEND_METHOD(SWFEditor, output);
ZEND_METHOD(SWFEditor, swfInfo);
ZEND_METHOD(SWFEditor, _destroy_and_exit);
ZEND_METHOD(SWFEditor, getHeaderInfo);
ZEND_METHOD(SWFEditor, setHeaderInfo);
ZEND_METHOD(SWFEditor, getTagList);
ZEND_METHOD(SWFEditor, getTagDetail);
ZEND_METHOD(SWFEditor, getTagData);
ZEND_METHOD(SWFEditor, replaceTagData);
ZEND_METHOD(SWFEditor, getTagDataByCID);
ZEND_METHOD(SWFEditor, replaceTagDataByCID);
ZEND_METHOD(SWFEditor, getTagContentsByCID);
ZEND_METHOD(SWFEditor, replaceTagContentsByCID);
ZEND_METHOD(SWFEditor, removeTag);
ZEND_METHOD(SWFEditor, printTagData);
ZEND_METHOD(SWFEditor, getShapeData);
ZEND_METHOD(SWFEditor, replaceShapeData);
ZEND_METHOD(SWFEditor, setShapeAdjustMode);
ZEND_METHOD(SWFEditor, getShapeIdListByBitmapRef);
ZEND_METHOD(SWFEditor, getBitmapSize);
ZEND_METHOD(SWFEditor, getJpegData);
ZEND_METHOD(SWFEditor, getJpegAlpha);
ZEND_METHOD(SWFEditor, replaceJpegData);
ZEND_METHOD(SWFEditor, getPNGData);
ZEND_METHOD(SWFEditor, replacePNGData);
ZEND_METHOD(SWFEditor, replaceGIFData);
ZEND_METHOD(SWFEditor, replaceBitmapData);
ZEND_METHOD(SWFEditor, convertBitmapDataToJpegTag);
ZEND_METHOD(SWFEditor, applyShapeMatrixFactor);
ZEND_METHOD(SWFEditor, applyShapeRectFactor);
ZEND_METHOD(SWFEditor, getSoundData);
ZEND_METHOD(SWFEditor, replaceMLDData);
ZEND_METHOD(SWFEditor, getEditString);
ZEND_METHOD(SWFEditor, replaceEditString);
ZEND_METHOD(SWFEditor, getActionData);
ZEND_METHOD(SWFEditor, disasmActionData);
ZEND_METHOD(SWFEditor, setActionVariables);
ZEND_METHOD(SWFEditor, replaceActionStrings);
ZEND_METHOD(SWFEditor, replaceMovieClip);
ZEND_METHOD(SWFEditor, setCompressLevel);
ZEND_METHOD(SWFEditor, rebuild);
ZEND_METHOD(SWFEditor, purgeUselessContents);
ZEND_METHOD(SWFEditor, isShapeTagData);
ZEND_METHOD(SWFEditor, isBitmapTagData);


static const zend_function_entry class_SWFEditor_methods[] = {
	ZEND_ME(SWFEditor, __construct, arginfo_class_SWFEditor___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, input, arginfo_class_SWFEditor_input, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, output, arginfo_class_SWFEditor_output, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, swfInfo, arginfo_class_SWFEditor_swfInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, _destroy_and_exit, arginfo_class_SWFEditor__destroy_and_exit, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getHeaderInfo, arginfo_class_SWFEditor_getHeaderInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, setHeaderInfo, arginfo_class_SWFEditor_setHeaderInfo, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getTagList, arginfo_class_SWFEditor_getTagList, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getTagDetail, arginfo_class_SWFEditor_getTagDetail, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getTagData, arginfo_class_SWFEditor_getTagData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceTagData, arginfo_class_SWFEditor_replaceTagData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getTagDataByCID, arginfo_class_SWFEditor_getTagDataByCID, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceTagDataByCID, arginfo_class_SWFEditor_replaceTagDataByCID, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getTagContentsByCID, arginfo_class_SWFEditor_getTagContentsByCID, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceTagContentsByCID, arginfo_class_SWFEditor_replaceTagContentsByCID, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, removeTag, arginfo_class_SWFEditor_removeTag, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, printTagData, arginfo_class_SWFEditor_printTagData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getShapeData, arginfo_class_SWFEditor_getShapeData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceShapeData, arginfo_class_SWFEditor_replaceShapeData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, setShapeAdjustMode, arginfo_class_SWFEditor_setShapeAdjustMode, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getShapeIdListByBitmapRef, arginfo_class_SWFEditor_getShapeIdListByBitmapRef, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getBitmapSize, arginfo_class_SWFEditor_getBitmapSize, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getJpegData, arginfo_class_SWFEditor_getJpegData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getJpegAlpha, arginfo_class_SWFEditor_getJpegAlpha, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceJpegData, arginfo_class_SWFEditor_replaceJpegData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getPNGData, arginfo_class_SWFEditor_getPNGData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replacePNGData, arginfo_class_SWFEditor_replacePNGData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceGIFData, arginfo_class_SWFEditor_replaceGIFData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceBitmapData, arginfo_class_SWFEditor_replaceBitmapData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, convertBitmapDataToJpegTag, arginfo_class_SWFEditor_convertBitmapDataToJpegTag, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, applyShapeMatrixFactor, arginfo_class_SWFEditor_applyShapeMatrixFactor, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, applyShapeRectFactor, arginfo_class_SWFEditor_applyShapeRectFactor, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getSoundData, arginfo_class_SWFEditor_getSoundData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceMLDData, arginfo_class_SWFEditor_replaceMLDData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getEditString, arginfo_class_SWFEditor_getEditString, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceEditString, arginfo_class_SWFEditor_replaceEditString, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, getActionData, arginfo_class_SWFEditor_getActionData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, disasmActionData, arginfo_class_SWFEditor_disasmActionData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, setActionVariables, arginfo_class_SWFEditor_setActionVariables, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceActionStrings, arginfo_class_SWFEditor_replaceActionStrings, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, replaceMovieClip, arginfo_class_SWFEditor_replaceMovieClip, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, setCompressLevel, arginfo_class_SWFEditor_setCompressLevel, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, rebuild, arginfo_class_SWFEditor_rebuild, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, purgeUselessContents, arginfo_class_SWFEditor_purgeUselessContents, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, isShapeTagData, arginfo_class_SWFEditor_isShapeTagData, ZEND_ACC_PUBLIC)
	ZEND_ME(SWFEditor, isBitmapTagData, arginfo_class_SWFEditor_isBitmapTagData, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_SWFEditor(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "SWFEditor", class_SWFEditor_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);

	zval const_SHAPE_BITMAP_NONE_value;
	ZVAL_LONG(&const_SHAPE_BITMAP_NONE_value, SWFED_SHAPE_BITMAP_NONE);
	zend_string *const_SHAPE_BITMAP_NONE_name = zend_string_init_interned("SHAPE_BITMAP_NONE", sizeof("SHAPE_BITMAP_NONE") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_SHAPE_BITMAP_NONE_name, &const_SHAPE_BITMAP_NONE_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_SHAPE_BITMAP_NONE_name);

	zval const_SHAPE_BITMAP_MATRIX_RESCALE_value;
	ZVAL_LONG(&const_SHAPE_BITMAP_MATRIX_RESCALE_value, SWFED_SHAPE_BITMAP_MATRIX_RESCALE);
	zend_string *const_SHAPE_BITMAP_MATRIX_RESCALE_name = zend_string_init_interned("SHAPE_BITMAP_MATRIX_RESCALE", sizeof("SHAPE_BITMAP_MATRIX_RESCALE") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_SHAPE_BITMAP_MATRIX_RESCALE_name, &const_SHAPE_BITMAP_MATRIX_RESCALE_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_SHAPE_BITMAP_MATRIX_RESCALE_name);

	zval const_SHAPE_BITMAP_RECT_RESIZE_value;
	ZVAL_LONG(&const_SHAPE_BITMAP_RECT_RESIZE_value, SWFED_SHAPE_BITMAP_RECT_RESIZE);
	zend_string *const_SHAPE_BITMAP_RECT_RESIZE_name = zend_string_init_interned("SHAPE_BITMAP_RECT_RESIZE", sizeof("SHAPE_BITMAP_RECT_RESIZE") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_SHAPE_BITMAP_RECT_RESIZE_name, &const_SHAPE_BITMAP_RECT_RESIZE_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_SHAPE_BITMAP_RECT_RESIZE_name);

	zval const_SHAPE_BITMAP_TYPE_TILLED_value;
	ZVAL_LONG(&const_SHAPE_BITMAP_TYPE_TILLED_value, SWFED_SHAPE_BITMAP_TYPE_TILLED);
	zend_string *const_SHAPE_BITMAP_TYPE_TILLED_name = zend_string_init_interned("SHAPE_BITMAP_TYPE_TILLED", sizeof("SHAPE_BITMAP_TYPE_TILLED") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_SHAPE_BITMAP_TYPE_TILLED_name, &const_SHAPE_BITMAP_TYPE_TILLED_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_SHAPE_BITMAP_TYPE_TILLED_name);

	return class_entry;
}
