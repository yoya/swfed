/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2006 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1 2006/01/01 12:50:00 sniper Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_swfed.h"

#include "swf_define.h"
#include "y_keyvalue.h"

#include "swf_tag_jpeg.h"
#include "swf_tag_lossless.h"
#include "swf_tag_edit.h"
#include "swf_tag_sound.h"
#include "swf_tag_action.h"
#include "swf_tag_sprite.h"
#include "swf_tag_shape.h"
#include "swf_tag_place.h"
#include "swf_tag.h"
#include "swf_object.h"

/* If you declare any globals in php_swfed.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(swfed)
*/

/* True global resources - no need for thread safety here */
static int le_swfed;

/* {{{ swfed_functions[]
 *
 * Every user visible function must have an entry in swfed_functions[].
 */
zend_function_entry swfed_functions[] = {
    PHP_ME(swfed,  __construct, NULL, 0)
    PHP_ME(swfed,  input, NULL, 0)
    PHP_ME(swfed,  output, NULL, 0)
    PHP_ME(swfed,  _destroy_and_exit, NULL, 0) // for debug
    PHP_ME(swfed,  getHeaderInfo, NULL, 0)
    PHP_ME(swfed,  setHeaderInfo, NULL, 0)
    PHP_ME(swfed,  getTagList, NULL, 0)
    PHP_ME(swfed,  getTagDetail, NULL, 0)
    PHP_ME(swfed,  getTagData, NULL, 0)
    PHP_ME(swfed,  replaceTagData, NULL, 0)
    PHP_ME(swfed,  getTagContentsByCID, NULL, 0)
    PHP_ME(swfed,  replaceTagContentsByCID, NULL, 0)
    PHP_ME(swfed,  getShapeData, NULL, 0)
    PHP_ME(swfed,  replaceShapeData, NULL, 0)
    
    PHP_ME(swfed,  setShapeAdjustMode, NULL, 0)
    PHP_ME(swfed,  getShapeIdListByBitmapRef, NULL, 0)
    PHP_ME(swfed,  getBitmapSize, NULL, 0)
    PHP_ME(swfed,  getJpegData, NULL, 0)
    PHP_ME(swfed,  getJpegAlpha, NULL, 0)
    PHP_ME(swfed,  replaceJpegData, NULL, 0)
    PHP_ME(swfed,  getPNGData, NULL, 0)
    PHP_ME(swfed,  replacePNGData, NULL, 0)
    PHP_ME(swfed,  replaceGIFData, NULL, 0)
    PHP_ME(swfed,  applyShapeMatrixFactor, NULL, 0)
    PHP_ME(swfed,  applyShapeRectFactor, NULL, 0)
    PHP_ME(swfed,  getSoundData, NULL, 0)
    PHP_ME(swfed,  replaceMLDData, NULL, 0)
    PHP_ME(swfed,  getEditString, NULL, 0)
    PHP_ME(swfed,  replaceEditString, NULL, 0)
    PHP_ME(swfed,  getActionData, NULL, 0)
    PHP_ME(swfed,  disasmActionData, NULL, 0)
    PHP_ME(swfed,  setActionVariables, NULL, 0)
    PHP_ME(swfed,  replaceMovieClip, NULL, 0)
    PHP_ME(swfed,  swfInfo, NULL, 0)
    {NULL, NULL, NULL}	/* Must be the last line in swfed_functions[] */
};
/* }}} */

#define SWFED_VERSION "0.35a"

/* {{{ swfed_module_entry
 */
zend_module_entry swfed_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"swfed",
	NULL, // no global functions
	PHP_MINIT(swfed),
	PHP_MSHUTDOWN(swfed),
	PHP_RINIT(swfed),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(swfed),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(swfed),
#if ZEND_MODULE_API_NO >= 20010901
	SWFED_VERSION, /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SWFED
ZEND_GET_MODULE(swfed)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("swfed.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_swfed_globals, swfed_globals)
    STD_PHP_INI_ENTRY("swfed.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_swfed_globals, swfed_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_swfed_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_swfed_init_globals(zend_swfed_globals *swfed_globals)
{
	swfed_globals->global_value = 0;
	swfed_globals->global_string = NULL;
}
*/
/* }}} */


static void free_swfed_resource(zend_rsrc_list_entry *resource TSRMLS_DC);
static swf_object_t  *get_swf_object(zval *obj TSRMLS_DC);

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(swfed)
{
	/* If you have INI entries, uncomment these lines 
	ZEND_INIT_MODULE_GLOBALS(swfed, php_swfed_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "SWFEditor", swfed_functions);
	swfeditor_ce = zend_register_internal_class(&ce TSRMLS_CC);
	le_swfed = zend_register_list_destructors_ex(free_swfed_resource, NULL, "SWFEditor", module_number);
    
	zend_declare_property_stringl(swfeditor_ce,
				"swf_object", strlen("swf_object"),
				"", 0, ZEND_ACC_PUBLIC TSRMLS_CC);
        // class const
        REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_NONE", SWFED_SHAPE_BITMAP_NONE);
        REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_MATRIX_RESCALE", SWFED_SHAPE_BITMAP_MATRIX_RESCALE);
        REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_RECT_RESIZE", SWFED_SHAPE_BITMAP_RECT_RESIZE);
        REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_TYPE_TILLED", SWFED_SHAPE_BITMAP_TYPE_TILLED);
	return SUCCESS;
}

/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(swfed)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(swfed)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(swfed)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(swfed)
{
	int gif_support = 0;
	int png_support = 0;
	php_info_print_table_start();
	php_info_print_table_header(2, "SWF Editor support", "enabled");
        php_info_print_table_row(2, "SWF Editor version", SWFED_VERSION);
#ifdef HAVE_PNG
	png_support = 1;
#endif
#ifdef HAVE_GIF
	gif_support = 1;
#endif
	php_info_print_table_row(2, "SWF Editor PNG support", png_support?"yes":"no");
	php_info_print_table_row(2, "SWF Editor GIF support", gif_support?"yes":"no");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_swfed_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_swfed_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char string[256];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = sprintf(string, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "swfed", arg);
// 	RETURN_STRINGL(string, len, 0);
    RETURN_STRINGL(string, len, 1);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

PHP_METHOD(swfed, __construct) {
    swf_object_t *swf = swf_object_open();
    int ret = 0;
    if (swf == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Couldn't create swf object");
    }
    ret = zend_list_insert(swf, le_swfed);
    object_init_ex(getThis(), swfeditor_ce);
    add_property_resource(getThis(), "swfed", ret);
    zend_list_addref(ret);
}

PHP_METHOD(swfed, input) {
    char *data = NULL;
    int data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_input(swf, (unsigned char *) data, data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, output) {
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (ZEND_NUM_ARGS() != 0) {
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_output(swf, &len);
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "output: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    free(data);
    RETURN_STRINGL(new_buff, len, 0);
}

PHP_METHOD(swfed, _destroy_and_exit) { // for debug
    swf_object_t *swf = NULL;
    swf = get_swf_object(getThis() TSRMLS_CC);
    swf_object_close(swf);
    exit(0);
}

PHP_METHOD(swfed, getHeaderInfo) {
    swf_object_t *swf = get_swf_object(getThis() TSRMLS_CC);
    array_init(return_value);
    if (memcmp(swf->header.magic, "CWS", 3) == 0) {
        add_assoc_bool(return_value, "compress", 1);
    } else { // FWS
        add_assoc_bool(return_value, "compress", 0);
    }
    add_assoc_long(return_value, "version", swf->header.version);
}

PHP_METHOD(swfed, setHeaderInfo) {
    zval *header_info = NULL;
    swf_object_t *swf = NULL;
    HashTable *header_table = NULL;
    zval **tmp = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                              &header_info) == FAILURE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid parameters");
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    header_table = Z_ARRVAL_P(header_info);
    if (zend_hash_find(header_table, "compress", sizeof("compress"), (void**)&tmp) == SUCCESS) {
        convert_to_boolean_ex(tmp);
        if (Z_LVAL_PP(tmp) != 0) {
            memcpy(swf->header.magic, "CWS", 3);
        } else {
            memcpy(swf->header.magic, "FWS", 3);
        }
    }

    if (zend_hash_find(header_table, "version", sizeof("version"), (void**)&tmp) == SUCCESS) {
        convert_to_long_ex(tmp);
        swf->header.version = Z_LVAL_PP(tmp);
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, getTagList) {
    int i = 0;
    zval *data = NULL;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    swf_tag_info_t *tag_info = NULL;
    if (ZEND_NUM_ARGS() != 0) {
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    tag = swf->tag;
    array_init(return_value);
    for (tag=swf->tag ; tag ; tag=tag->next) {
        ALLOC_INIT_ZVAL(data);
        array_init(data);
        add_assoc_long(data, "tag", tag->tag);
        tag_info = get_swf_tag_info(tag->tag);
        if (tag_info && tag_info->name) {
            add_assoc_string_ex(data,
                                "tagName", sizeof("tagName"),
                                (char *)tag_info->name, 1);
        }
        add_assoc_long(data, "length", tag->length);
        if (tag_info && tag_info->detail_handler) {
            add_assoc_bool(data, "detail", 1);
        }
        add_index_zval(return_value, i, data);
        i++;
    }
}

PHP_METHOD(swfed, getTagDetail) {
    long tag_seqno = 0;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    swf_tag_info_t *tag_info = NULL;
    int i = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    i = 0;
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (i == tag_seqno) {
            break;
        }
        i++;
    }
    if (tag == NULL) {
        RETURN_FALSE;
    }
    tag_info = get_swf_tag_info(tag->tag);
    if ((tag_info == NULL) || (tag_info->detail_handler == NULL)) {
        RETURN_FALSE;
    }
    if (swf_tag_create_input_detail(tag, swf) == NULL) {
        RETURN_FALSE; // can't create detail
    }
    switch (tag->tag) {
        swf_tag_jpeg_detail_t     *tag_jpeg;
        swf_tag_lossless_detail_t *tag_lossless;
        swf_tag_edit_detail_t     *tag_edit;
        swf_tag_sound_detail_t    *tag_sound;
        swf_tag_action_detail_t   *tag_action;
        swf_tag_sprite_detail_t   *tag_sprite;
        swf_tag_shape_detail_t    *tag_shape;
        swf_tag_place_detail_t    *tag_place;
      case 6:  // DefineBitsJPEG
      case 21: // DefineBitsJPEG2
      case 35: // DefineBitsJPEG3
        tag_jpeg = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "image_id", tag_jpeg->image_id);
        add_assoc_long(return_value, "jpeg_data_len", tag_jpeg->jpeg_data_len);
        if ((tag_jpeg->alpha_data != NULL) &&
            (tag_jpeg->alpha_data_len > 0)) {
            add_assoc_long(return_value, "alpha_data_len", tag_jpeg->alpha_data_len);
        }
        break;
      case 20: // DefineBitsLossless
      case 36: // DefineBitsLossless2
        tag_lossless = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "image_id", tag_lossless->image_id);
        add_assoc_long(return_value, "format", tag_lossless->format);
        add_assoc_long(return_value, "width", tag_lossless->width);
        add_assoc_long(return_value, "height", tag_lossless->height);
        if (tag_lossless->format == 3) {
            add_assoc_long(return_value, "colormap_count", tag_lossless->colormap_count);
        }
        break;
    case 14: // DefineSound
        tag_sound = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "sound_id", tag_sound->sound_id);
        add_assoc_long(return_value, "format", (unsigned long) tag_sound->sound_format);
        add_assoc_long(return_value, "rate", (unsigned long) tag_sound->sound_rate);
        add_assoc_long(return_value, "is_16bits", tag_sound->sound_is_16bits?1:0);
        add_assoc_long(return_value, "is_stereo", tag_sound->sound_is_stereo?1:0);
        add_assoc_long(return_value, "sound_samples_count", tag_sound->sound_samples_count);
        add_assoc_long(return_value, "sound_data_len", tag_sound->sound_data_len);
        break;
    case 12: // DoAction
    case 59: // DoInitAction
        tag_action = tag->detail;
        array_init(return_value);
        if (tag->tag == 59) { // DoInitAction
            add_assoc_long(return_value, "action_sprite", tag_action->action_sprite);
        }
        if (tag_action->action_record && tag_action->action_record_len) {
            add_assoc_long(return_value, "action_record_len", tag_action->action_record_len);
        }
        break;
      case 37: // DefineEditText;
        tag_edit = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "edit_id", tag_edit->edit_id);
        if (tag_edit->edit_variable_name && tag_edit->edit_variable_name[0]) {
            add_assoc_string_ex(return_value, "variable_name",
                                sizeof("variable_name"),
                                (char *)tag_edit->edit_variable_name, 1);
        }
        if (tag_edit->edit_initial_text && tag_edit->edit_initial_text[0]) {
            add_assoc_string_ex(return_value, "initial_text",
                                sizeof("initial_text"),
                                (char *)tag_edit->edit_initial_text, 1);
        }
        break;
      case 39: // DefineSprite;
        tag_sprite = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "sprite_id", tag_sprite->sprite_id);
        add_assoc_long(return_value, "frame_count", tag_sprite->frame_count);
        break;
      case 2: // DefineShape;
      case 22: // DefineShape2;
      case 32: // DefineShape3;
      case 46: // DefineMorphShape;
        tag_shape = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "shape_id", tag_shape->shape_id);
        add_assoc_long(return_value, "fill_styles.count", tag_shape->shape_with_style.styles.fill_styles.count);
        add_assoc_long(return_value, "line_styles.count", tag_shape->shape_with_style.styles.line_styles.count);
//        tag_shape->shape_with_style.shape_records
        break;
      case 4: // PlaceObject
      case 26: // PlaceObject2
        tag_place = tag->detail;
        array_init(return_value);
        if ((tag->tag == 4) || tag_place->flag_has_character) {
            add_assoc_long(return_value, "character_id", tag_place->character_id);
        }
        add_assoc_long(return_value, "depth", tag_place->depth);
        if (tag_place->flag_has_name) {
            add_assoc_string_ex(return_value, "name",
                                sizeof("name"), (char *)tag_place->name, 1);
        }

      default:
          break;
        RETURN_FALSE;
    }
    /* return_value */
}

PHP_METHOD(swfed, getTagData) {
    long tag_seqno = 0;
    swf_object_t *swf = NULL;
    unsigned char *data_ref = NULL;
    unsigned long data_len = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data_ref = swf_object_get_tagdata(swf, tag_seqno, &data_len);
    if (data_ref == NULL) {
        fprintf(stderr, "getTagData: Can't get_tagdata\n");
        RETURN_FALSE;
    }
    RETURN_STRINGL(data_ref, data_len, 1);
}

PHP_METHOD(swfed, replaceTagData) {
    char *data = NULL;
    unsigned long data_len = 0;
    long tag_seqno = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &tag_seqno, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_tagdata(swf, tag_seqno,
                                        (unsigned char *)data, data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, getTagContentsByCID) {
    long cid = 0;
    swf_object_t *swf = NULL;
    unsigned char *data_ref = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data_ref = swf_object_get_tagcontents_bycid(swf, cid, &data_len);
    if (data_ref == NULL) {
        fprintf(stderr, "getTagContentsByCID: Can't get_tagcontents_bycid\n");
        RETURN_FALSE;
    }
    RETURN_STRINGL(data_ref, data_len, 1);
}

PHP_METHOD(swfed, replaceTagContentsByCID) {
    char *data = NULL;
    unsigned long data_len = 0;
    long cid = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &cid, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_tagcontents_bycid(swf, cid,
                                                  (unsigned char *)data,
                                                  data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}


PHP_METHOD(swfed, getShapeData) {
    long cid = 0;
    swf_object_t *swf = NULL;
    unsigned char *data_ref = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data_ref = swf_object_get_shapedata(swf, cid, &data_len);
    if (data_ref == NULL) {
        fprintf(stderr, "getShapeData: Can't get_tagcontents_bycid\n");
        RETURN_FALSE;
    }
    RETURN_STRINGL(data_ref, data_len, 1);
}

PHP_METHOD(swfed, replaceShapeData) {
    char *data = NULL;
    unsigned long data_len = 0;
    long cid = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &cid, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_shapedata(swf, cid,
                                          (unsigned char *)data,
                                          data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, setShapeAdjustMode) {
    swf_object_t *swf = NULL;
    unsigned long mode = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &mode) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    swf_object_set_shape_adjust_mode(swf, mode);
    RETURN_TRUE;
}

PHP_METHOD(swfed, getShapeIdListByBitmapRef) {
    long bitmap_id;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    int i;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &bitmap_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    array_init(return_value);
    i = 0;
    for (tag = swf->tag ; tag ; tag=tag->next) {
        register int tag_code = tag->tag;
        if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_get_refcid(tag) == bitmap_id)) {
            swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
            add_index_long(return_value, i, (long) swf_tag_shape->shape_id);
            i++;
        }
    }
}

PHP_METHOD(swfed, getBitmapSize) {
    long bitmap_id;
    swf_object_t *swf = NULL;
    int width, height;
    int ret;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &bitmap_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    ret = swf_object_get_bitmap_size(swf, bitmap_id, &width, &height);
    if (ret) {
        RETURN_FALSE;
    }
    array_init(return_value);
    add_assoc_long(return_value, "width", (long) width);
    add_assoc_long(return_value, "height", (long) height);
}

PHP_METHOD(swfed, getJpegData) {
    unsigned long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_get_jpegdata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "getJpegData Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    free(data);
    RETURN_STRINGL(new_buff, (int) len, 0);
}

PHP_METHOD(swfed, getJpegAlpha) {
    unsigned long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_get_alphadata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "getJpegAlpha Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    free(data);
    RETURN_STRINGL(new_buff, len, 0);
}

PHP_METHOD(swfed, replaceJpegData) {
    char *data = NULL, *alpha_data = NULL;
    int data_len = 0 , alpha_data_len = 0;
    int image_id = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &image_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
         break;
      case 3:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lss", &image_id, &data, &data_len, &alpha_data, &alpha_data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_jpegdata(swf, image_id,
                                         (unsigned char *)data,
                                         (unsigned long) data_len,
                                         (unsigned char *)alpha_data,
                                         (unsigned long) alpha_data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, getPNGData) {
#ifndef HAVE_PNG
    fprintf(stderr, "replacePNGData: no png library\n");
    RETURN_FALSE;
#else /* HAVE_PNG */
    unsigned long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_get_pngdata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "getPNGData: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    free(data);
    RETURN_STRINGL(new_buff, (int) len, 0);
#endif /* HAVE_PNG */
}

PHP_METHOD(swfed, replacePNGData) {
#ifndef HAVE_PNG
    fprintf(stderr, "replacePNGData: no png library\n");
    RETURN_FALSE;
#else  /* HAVE_PNG */
    char *data = NULL;
    int data_len = 0;
    int image_id = 0;
    swf_object_t *swf;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &image_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_pngdata(swf, image_id,
                                        (unsigned char *)data,
                                        (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
#endif /* HAVE_PNG */
}

PHP_METHOD(swfed, replaceGIFData) {
#ifndef HAVE_GIF
    fprintf(stderr, "replaceGIFData: no gif library\n");
    RETURN_FALSE;
#else /* HAVE_GIF */
    char *data = NULL;
    int data_len = 0;
    int image_id = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &image_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_gifdata(swf, image_id,
                                        (unsigned char *)data,
                                        (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
#endif /* HAVE_GIF */
}

PHP_METHOD(swfed, applyShapeMatrixFactor) {
    long shape_id = 0;
    double scale_x = 1, scale_y = 1, rotate_rad = 0;
    long trans_x = 0, trans_y = 0;
    swf_object_t *swf = NULL;
    int result;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "ldddll", &shape_id, &scale_x, &scale_y,
                              &rotate_rad, &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_apply_shapematrix_factor(swf, shape_id,
                                                 scale_x, scale_y, rotate_rad,
                                                 trans_x, trans_y);
    if (result) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(swfed, applyShapeRectFactor) {
    long shape_id = 0;
    double scale_x = 1, scale_y = 1;
    long trans_x = 0, trans_y = 0;
    swf_object_t *swf = NULL;
    int result;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "lddll", &shape_id, &scale_x, &scale_y,
                              &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_apply_shaperect_factor(swf, shape_id,
                                                 scale_x, scale_y,
                                                 trans_x, trans_y);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

/*
PHP_METHOD(swfed, adjustShapeSizeToBitmap) {
    RETURN_TRUE;
}

PHP_METHOD(swfed, adjustShapeScaleToBitmap) {
    RETURN_TRUE;
}
*/

PHP_METHOD(swfed, getSoundData) {
    unsigned long sound_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "l", &sound_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_get_sounddata(swf, &len, sound_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "getSoundData: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    free(data);
    RETURN_STRINGL(new_buff, (int) len, 0);
}

PHP_METHOD(swfed, replaceMLDData) {
    char *data = NULL;
    int data_len = 0;
    int sound_id = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &sound_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    result = swf_object_replace_melodata(swf, sound_id,
                                         (unsigned char *)data,
                                         (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, getEditString) {
    char *var_name = NULL;
    int  var_name_len = 0;
    swf_object_t *swf = NULL;
    char *data = NULL, *new_buff = NULL;
    int str_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &var_name, &var_name_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data = swf_object_get_editstring(swf, var_name, var_name_len);
    if (data == NULL) {
        RETURN_FALSE;
    }
    str_len = strlen(data);
    new_buff = emalloc(str_len);
    if (new_buff == NULL) {
        fprintf(stderr, "getEditString: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, str_len);
    free(data);
    RETURN_STRINGL(new_buff, str_len, 0);
}

PHP_METHOD(swfed, replaceEditString) {
    char *var_name = NULL, *ini_text = NULL;
    int  var_name_len = 0, ini_text_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &var_name, &var_name_len,
                              &ini_text, &ini_text_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);    
    result = swf_object_replace_editstring(swf, var_name,  var_name_len,
                                           ini_text, ini_text_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, getActionData) {
    long tag_seqno = 0;
    swf_object_t *swf = NULL;
    unsigned char *data_ref = NULL;
    char *new_buff = NULL;
    unsigned long data_len = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);
    data_ref = swf_object_get_actiondata(swf, &data_len, tag_seqno);
    if (data_ref == NULL) {
        fprintf(stderr, "getActionData: Can't get_actiondata\n");
        RETURN_FALSE;
    }
    new_buff = emalloc(data_len);
    if (new_buff == NULL) {
        fprintf(stderr, "getActionData: Can't emalloc new_buff\n");
        RETURN_FALSE;
    }
    memcpy(new_buff, data_ref, data_len);
    RETURN_STRINGL(new_buff, data_len, 0);
}

PHP_METHOD(swfed, disasmActionData) {
    char *data = NULL;
    int data_len = 0;
    bitstream_t *bs = NULL;
    swf_action_list_t *action_list = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    array_init(return_value);
    bs = bitstream_open();
    bitstream_input(bs, (unsigned char*) data, data_len);
    action_list = swf_action_list_create(bs);
    bitstream_close(bs);
    if (action_list) {
        swf_action_t *action = action_list->head;
        while(action) {
            printf("\t");
//            swf_action_disasm(action);
            action = action->next;
        }
    }
    swf_action_list_destroy(action_list);
    RETURN_TRUE;
}

PHP_METHOD(swfed, setActionVariables) {
    zval *zid, *arr, **entry;
    HashTable *arr_hash;
    HashPosition    pos;
    char            *str_key, *str_value;
    uint            str_key_len, str_value_len;
    ulong tmp;
    char tmp_str[17];
    int ret;
    y_keyvalue_t *kv;
    swf_object_t *swf = get_swf_object(getThis() TSRMLS_CC);
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &arr) == FAILURE) {
        RETURN_FALSE;
    }
    kv = y_keyvalue_open();

    arr_hash = Z_ARRVAL_P(arr);
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(arr), &pos);
    while (zend_hash_get_current_data_ex(Z_ARRVAL_P(arr), (void **)&entry, &
                                         pos) == SUCCESS) {
        str_value = Z_STRVAL_PP(entry);
        str_value_len = Z_STRLEN_PP(entry);
        ret = zend_hash_get_current_key_ex(Z_ARRVAL_P(arr), &str_key, &
                                           str_key_len, &tmp, 0, &pos);
        switch (ret) {
        case HASH_KEY_IS_STRING:
            y_keyvalue_set(kv, str_key, str_key_len - 1, str_value, str_value_len);
            break;
        case HASH_KEY_IS_LONG:
            snprintf(tmp_str, 17, "%ld\0", tmp);
            y_keyvalue_set(kv, tmp_str, strlen(tmp_str), str_value, str_value_len);
            break;
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Array keys invalid type(%d).", ret);
        }
        zend_hash_move_forward_ex(Z_ARRVAL_P(arr), &pos);
    }
    swf_object_insert_action_setvariables(swf, kv);
    y_keyvalue_close(kv);
    RETURN_TRUE;
}

PHP_METHOD(swfed, replaceMovieClip) {
    char *instance_name = NULL, *swf_data = NULL;
    int  instance_name_len = 0, swf_data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
                              &instance_name, &instance_name_len,
                              &swf_data, &swf_data_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis() TSRMLS_CC);    
    result = swf_object_replace_movieclip(swf, instance_name,
                                          instance_name_len,
                                          swf_data, swf_data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(swfed, swfInfo) {
    swf_object_t *swf = get_swf_object(getThis() TSRMLS_CC);
    swf_object_print(swf);
    RETURN_TRUE;
}

static swf_object_t  *get_swf_object(zval *obj TSRMLS_DC) {
//    zval *data, **tmp;
    zval **tmp = NULL;
    swf_object_t *swf = NULL;
    int id = 0, type = 0;
/* XXX: zend_read_property 
    data = zend_read_property(Z_OBJCE_P(obj), obj, "swf_object",
							  strlen("swf_object"), 1 TSRMLS_CC);
*/
    if (zend_hash_find(Z_OBJPROP_P(obj), "swfed", strlen("swfed") + 1,
                       (void **)&tmp) == FAILURE) {
        return NULL;
    }
    id = Z_LVAL_PP(tmp);
    swf = (swf_object_t *) zend_list_find(id, &type);
    return swf;
}

static void free_swfed_resource(zend_rsrc_list_entry *resource TSRMLS_DC)
{
//    printf("SWFEditor->destory\n");
    swf_object_close((swf_object_t *) resource->ptr);
}
