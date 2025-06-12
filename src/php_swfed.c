/*
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Yoshihiro Yamazaki <yoya@awm.jp>                             |
  +----------------------------------------------------------------------+
*/

/* $Id:$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_swfed.h"

#include "swf_define.h"
#include "y_keyvalue.h"
#include "bitmap_util.h"

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

#if PHP_VERSION_ID < 80000
#include "php_swfed_legacy_arginfo.h"
#else
#include "php_swfed_arginfo.h"
#endif

#define get_zend_hash_value_long(table, key, value) do { \
        zval *tmp = NULL; \
        if ((tmp = zend_hash_str_find(table, ZEND_STRL(key))) != NULL) { \
            convert_to_long_ex(tmp); \
            value = Z_LVAL_P(tmp); \
        } \
    } while (0);

#define get_zend_hash_value_boolean(table, key, value) do { \
        zval *tmp = NULL; \
        if ((tmp = zend_hash_str_find(table, ZEND_STRL(key))) != NULL) { \
            convert_to_boolean_ex(tmp); \
            value = Z_LVAL_P(tmp); \
        } \
    } while (0);

static zend_class_entry *swfed_entry_ptr = NULL;

static int
param_is_null(int n) {
    zval *p;
    zend_parse_parameters(n, "z", &p);
    if(Z_TYPE_P(p) == IS_NULL) {
        return 1; // true
    }
    return 0; // false
}


/* If you declare any globals in php_swfed.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(swfed)
*/

/* True global resources - no need for thread safety here */
static int le_swfed;

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
        PHP_SWFED_VERSION,
#endif
        STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SWFED
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
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


static void free_swfed_resource(zend_resource *resource);
static swf_object_t  *get_swf_object(zval *obj);

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(swfed)
{
    /* If you have INI entries, uncomment these lines
       ZEND_INIT_MODULE_GLOBALS(swfed, php_swfed_init_globals, NULL);
       REGISTER_INI_ENTRIES();
       */
    swfed_entry_ptr = register_class_SWFEditor();
    #if ZEND_MODULE_API_NO >= 20220829 // php 8.2
    swfed_entry_ptr->ce_flags |= ZEND_ACC_ALLOW_DYNAMIC_PROPERTIES;
    #endif
    le_swfed = zend_register_list_destructors_ex(free_swfed_resource, NULL, "SWFEditor", module_number);
    // REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_NONE", SWFED_SHAPE_BITMAP_NONE);
    // REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_MATRIX_RESCALE", SWFED_SHAPE_BITMAP_MATRIX_RESCALE);
    // REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_RECT_RESIZE", SWFED_SHAPE_BITMAP_RECT_RESIZE);
    // REGISTER_SWFED_CLASS_CONST_LONG("SHAPE_BITMAP_TYPE_TILLED", SWFED_SHAPE_BITMAP_TYPE_TILLED);
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
#if defined(COMPILE_DL_EXTNAME) && defined(ZTS)
        ZEND_TSRMLS_CACHE_UPDATE();
#endif
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
        php_info_print_table_row(2, "SWF Editor version", PHP_SWFED_VERSION);
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
        size_t arg_len, len;
        zend_string *string = zend_string_alloc(255, 0);

        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
                return;
        }

        len = sprintf(string->val, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "swfed", arg);
//         RETURN_STRINGL(string, len, 0);
        RETURN_STR(string);
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

PHP_METHOD(SWFEditor, __construct) {
    //printf("__construct\n");
    swf_object_t *swf = swf_object_open();
    zval *ret = 0;
    if (swf == NULL) {
        php_error_docref(NULL, E_ERROR, "Couldn't create swf object");
    }
    ret = zend_list_insert(swf, le_swfed);
    //object_init_ex(getThis(), swfeditor_ce);
    add_property_resource(getThis(), "swf_object", Z_RES_P(ret));
    Z_ADDREF_P(ret);
}

PHP_METHOD(SWFEditor, input) {
    char *data = NULL;
    size_t data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    result = swf_object_input(swf, (unsigned char *) data, data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, output) {
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (ZEND_NUM_ARGS() != 0) {
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
    }
    swf = get_swf_object(getThis());
    data = swf_object_output(swf, &len);
    new_buff = emalloc(len);
    if (new_buff == NULL) {
        fprintf(stderr, "output: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, len);
    //free(data);
    //RETURN_STRINGL(new_buff, len, 0);
    zend_string *str = zend_string_init(new_buff, len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, swfInfo) {
    swf_object_t *swf = get_swf_object(getThis());
    swf_object_print(swf);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, _destroy_and_exit) { // for debug
    swf_object_t *swf = NULL;
    swf = get_swf_object(getThis());
    swf_object_close(swf);
    exit(0);
}

PHP_METHOD(SWFEditor, getHeaderInfo) {
    swf_object_t *swf = get_swf_object(getThis());
    array_init(return_value);
    if (memcmp(swf->header.magic, "CWS", 3) == 0) {
        add_assoc_bool(return_value, "compress", 1);
    } else { // FWS
        add_assoc_bool(return_value, "compress", 0);
    }
    add_assoc_long(return_value, "version", swf->header.version);

    add_assoc_long(return_value, "x_min", swf->header_movie.frame_size.x_min / SWF_TWIPS);
    add_assoc_long(return_value, "y_min", swf->header_movie.frame_size.y_min / SWF_TWIPS);
    add_assoc_long(return_value, "x_max", swf->header_movie.frame_size.x_max / SWF_TWIPS);
    add_assoc_long(return_value, "y_max", swf->header_movie.frame_size.y_max / SWF_TWIPS);


}

PHP_METHOD(SWFEditor, setHeaderInfo) {
    zval *header_info = NULL;
    swf_object_t *swf = NULL;
    HashTable *header_table = NULL;
    zval *tmp = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a",
                              &header_info) == FAILURE) {
        php_error_docref(NULL, E_ERROR, "Invalid parameters");
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    header_table = Z_ARRVAL_P(header_info);

    // FWS or CWS
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("compress"))) == NULL) {
        convert_to_boolean_ex(tmp);
        if (Z_LVAL_P(tmp) != 0) {
            memcpy(swf->header.magic, "CWS", 3);
        } else {
            memcpy(swf->header.magic, "FWS", 3);
        }
    }

    // Version
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("version"))) == NULL) {
        convert_to_long_ex(tmp);
        swf->header.version = Z_LVAL_P(tmp);
    }

    // FrameRect
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("x_min"))) == NULL) {
        convert_to_long_ex(tmp);
        swf->header_movie.frame_size.x_min = Z_LVAL_P(tmp) * SWF_TWIPS;
    }
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("y_min"))) == NULL) {
        convert_to_long_ex(tmp);
        swf->header_movie.frame_size.y_min = Z_LVAL_P(tmp) * SWF_TWIPS;
    }
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("x_max"))) == NULL) {
        convert_to_long_ex(tmp);
        swf->header_movie.frame_size.x_max = Z_LVAL_P(tmp) * SWF_TWIPS;
    }
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("y_max"))) == NULL) {
        convert_to_long_ex(tmp);
        swf->header_movie.frame_size.y_max = Z_LVAL_P(tmp) * SWF_TWIPS;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getTagList) {
    int i = 0;
    zval data;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    swf_tag_info_t *tag_info = NULL;
    if (ZEND_NUM_ARGS() != 0) {
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
    }
    swf = get_swf_object(getThis());
    array_init(return_value);
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        array_init(&data);
        add_assoc_long(&data, "code", tag->code);
        add_assoc_long(&data, "tag", tag->code);
        tag_info = get_swf_tag_info(tag->code);
        if (tag_info && tag_info->name) {
            add_assoc_string_ex(&data,
                                "tagName", sizeof("tagName")-1,
                                (char *)tag_info->name);
        }
        add_assoc_long(&data, "length", tag->length);
        if (tag_info && tag_info->detail_handler) {
            add_assoc_bool(&data, "detail", 1);
        }
        add_index_zval(return_value, i, &data);
        i++;
    }
}

PHP_METHOD(SWFEditor, getTagDetail) {
    zend_long tag_seqno = 0;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    swf_tag_info_t *tag_info = NULL;
    int i = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    i = 0;
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (i == tag_seqno) {
            break;
        }
        i++;
    }
    if (tag == NULL) {
        RETURN_FALSE;
    }
    tag_info = get_swf_tag_info(tag->code);
    if ((tag_info == NULL) || (tag_info->detail_handler == NULL)) {
        RETURN_FALSE;
    }
    if (swf_tag_create_input_detail(tag, swf) == NULL) {
        RETURN_FALSE; // can't create detail
    }
    switch (tag->code) {
        swf_tag_jpeg_detail_t     *tag_jpeg;
        swf_tag_lossless_detail_t *tag_lossless;
        swf_tag_edit_detail_t     *tag_edit;
        swf_tag_sound_detail_t    *tag_sound;
        swf_tag_action_detail_t   *tag_action;
        swf_tag_sprite_detail_t   *tag_sprite;
        swf_tag_shape_detail_t    *tag_shape;
        swf_tag_place_detail_t    *tag_place;
        swf_action_t   *action;
        int action_list_count;
        zval data;
        int *bitmap_id_list, bitmap_id_list_num;
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
        if (tag->code == 59) { // DoInitAction
            add_assoc_long(return_value, "action_sprite", tag_action->action_sprite);
        }
        action = tag_action->action_list->head;
        if (action) {
            for (action_list_count = 0 ; action ; action_list_count++) {
                action = action->next;
            }
            add_assoc_long(return_value, "action_list_count", action_list_count);
        }
        break;
      case 37: // DefineEditText;
        tag_edit = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "edit_id", tag_edit->edit_id);
        if (tag_edit->edit_variable_name && tag_edit->edit_variable_name[0]) {
            add_assoc_string_ex(return_value, "variable_name",
                                sizeof("variable_name"),
                                (char *)tag_edit->edit_variable_name);
        }
        if (tag_edit->edit_initial_text && tag_edit->edit_initial_text[0]) {
            add_assoc_string_ex(return_value, "initial_text",
                                sizeof("initial_text"),
                                (char *)tag_edit->edit_initial_text);
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
        bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
        if (bitmap_id_list) {
            int i;
            array_init(&data);
            for (i = 0 ; i < bitmap_id_list_num ; i++) {
                add_index_long(&data, i , bitmap_id_list[i]);
            }
            add_assoc_zval(return_value, "bitmap_ref", &data);
            free(bitmap_id_list);
        }
        break;
      case 4: // PlaceObject
      case 26: // PlaceObject2
        tag_place = tag->detail;
        array_init(return_value);
        if ((tag->code == 4) || tag_place->flag_has_character) {
            add_assoc_long(return_value, "character_id", tag_place->character_id);
        }
        add_assoc_long(return_value, "depth", tag_place->depth);
        if (tag_place->flag_has_name) {
            add_assoc_string_ex(return_value, "name",
                                sizeof("name"), (char *)tag_place->name);
        }

      default:
          break;
        RETURN_FALSE;
    }
    /* return_value */
}

PHP_METHOD(SWFEditor, getTagData) {
    zend_long tag_seqno = 0;
    swf_object_t *swf = NULL;
    unsigned char *data = NULL, *new_buff;
    unsigned long data_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data = swf_object_get_tagdata(swf, tag_seqno, &data_len);

    if (data == NULL) {
        fprintf(stderr, "getTagData: Can't get_tagdata\n");
        RETURN_FALSE;
    }
    new_buff = emalloc(data_len);
    memcpy(new_buff, data, data_len);
    //RETURN_STRINGL(new_buff, data_len, 0);
    zend_string *str = zend_string_init((char *)new_buff, data_len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceTagData) {
    char *data = NULL;
    size_t data_len = 0;
    zend_long tag_seqno = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &tag_seqno, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_tagdata(swf, tag_seqno,
                                        (unsigned char *)data,
                                        (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getTagDataByCID) {
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    char *data_ref = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data_ref = (char *) swf_object_get_tagdata_bycid(swf, cid, &data_len);

    if (data_ref == NULL) {
        fprintf(stderr, "getTagDataByCID: Can't get_tagdata_bycid\n");
        RETURN_FALSE;
    }

    zend_string *str = zend_string_init(data_ref, data_len, 0);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceTagDataByCID) {
    char *data = NULL;
    size_t data_len = 0;
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &cid, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_tagdata_bycid(swf, cid,
                                              (unsigned char *)data,
                                              data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getTagContentsByCID) {
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    char *data_ref = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data_ref = (char *) swf_object_get_tagcontents_bycid(swf, cid, &data_len);

    if (data_ref == NULL) {
        fprintf(stderr, "getTagContentsByCID: Can't get_tagcontents_bycid\n");
        RETURN_FALSE;
    }
    //RETURN_STRINGL((char *)data_ref, data_len, 1);

    zend_string *str = zend_string_init(data_ref, data_len, 0);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceTagContentsByCID) {
    char *data = NULL;
    size_t data_len = 0;
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &cid, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_tagcontents_bycid(swf, cid,
                                                  (unsigned char *)data,
                                                  data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, removeTag) {
    zend_long tag_seqno = 0;
    zend_long tag_seqno_in_sprite = -1;
    swf_object_t *swf = NULL;
    int ret;

    switch (ZEND_NUM_ARGS()) {
      case 1:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &tag_seqno) == FAILURE) {
            RETURN_FALSE;
        }
        break;
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &tag_seqno, &tag_seqno_in_sprite) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    ret = swf_object_remove_tag(swf, tag_seqno, tag_seqno_in_sprite);
    if (ret) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, printTagData) {
    char *data = NULL;
    size_t data_len = 0;
    swf_object_t *swf = NULL;
    int ret = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 1:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    ret = swf_object_print_tagdata(swf, (unsigned char *)data,
                                   (unsigned long) data_len);
    if (ret) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getShapeData) {
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data = swf_object_get_shapedata(swf, cid, &data_len);

    if (data == NULL) {
        fprintf(stderr, "getShapeData: Can't swf_object_get_shapedata (cid=%ld)\n", (long)cid);
        RETURN_FALSE;
    }
    new_buff = emalloc(data_len);
    memcpy(new_buff, data, data_len);
    //RETURN_STRINGL((char *)new_buff, data_len, 0);

    zend_string *str = zend_string_init(new_buff, data_len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceShapeData) {
    char *data = NULL;
    size_t data_len = 0;
    zend_long cid = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &cid, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_shapedata(swf, cid,
                                          (unsigned char *)data,
                                          (unsigned long)data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, setShapeAdjustMode) {
    swf_object_t *swf = NULL;
    zend_long mode = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &mode) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    swf_object_set_shape_adjust_mode(swf, mode);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getShapeIdListByBitmapRef) {
    zend_long bitmap_id;
    int *bitmap_id_list, bitmap_id_list_num;
    swf_object_t *swf = NULL;
    swf_tag_t *tag = NULL;
    int i;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &bitmap_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    array_init(return_value);
    i = 0;
    for (tag = swf->tag_head ; tag ; tag=tag->next) {
        register int tag_code = tag->code;
        if (isShapeTag(tag_code)) {
            bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
            if (bitmap_id_list) {
                int j;
                for (j=0 ; j < bitmap_id_list_num ; j++) {
                    if (bitmap_id_list[j] == bitmap_id) {
                        swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                        add_index_long(return_value, i, (long) swf_tag_shape->shape_id);
                        i++;
                        break;
                    }
                }
                free(bitmap_id_list);
            }
        }
    }
}

PHP_METHOD(SWFEditor, getBitmapSize) {
    zend_long bitmap_id;
    swf_object_t *swf = NULL;
    int width, height;
    int ret;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &bitmap_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    ret = swf_object_get_bitmap_size(swf, bitmap_id, &width, &height);
    if (ret) {
        RETURN_FALSE;
    }
    array_init(return_value);
    add_assoc_long(return_value, "width", (long) width);
    add_assoc_long(return_value, "height", (long) height);
}

PHP_METHOD(SWFEditor, getJpegData) {
    zend_long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
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
    //RETURN_STRINGL(new_buff, (int) len, 0);

    zend_string *str = zend_string_init(new_buff, len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, getJpegAlpha) {
    zend_long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
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
    //RETURN_STRINGL(new_buff, len, 0);

    zend_string *str = zend_string_init(new_buff, len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceJpegData) {
    char *data = NULL, *alpha_data = NULL;
    size_t data_len = 0 , alpha_data_len = 0;
    zend_long image_id = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &image_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
         break;
      case 3:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "lss", &image_id, &data, &data_len, &alpha_data, &alpha_data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_jpegdata(swf, image_id,
                                         (unsigned char *)data,
                                         (unsigned long) data_len,
                                         (unsigned char *)alpha_data,
                                         (unsigned long) alpha_data_len,
                                         0);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getPNGData) {
#ifndef HAVE_PNG
    fprintf(stderr, "replacePNGData: no png library\n");
    RETURN_FALSE;
#else /* HAVE_PNG */
    zend_long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
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
    //RETURN_STRINGL(new_buff, (int) len, 0);
    zend_string *str = zend_string_init(new_buff, len, 0);
    free(data);
    RETURN_STR(str);
#endif /* HAVE_PNG */
}

PHP_METHOD(SWFEditor, replacePNGData) {
#ifndef HAVE_PNG
    fprintf(stderr, "replacePNGData: no png library\n");
    RETURN_FALSE;
#else  /* HAVE_PNG */
    zend_long image_id = 0;
    char *data = NULL;
    size_t data_len = 0;
    zval *opts = NULL;
    HashTable *opts_table = NULL;
    int rgb15 = -1;
    swf_object_t *swf;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
      case 3:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls|a", &image_id, &data, &data_len, &opts) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    if (opts) {
        opts_table = Z_ARRVAL_P(opts);
        get_zend_hash_value_boolean(opts_table, "rgb15",  rgb15);
    }

    swf = get_swf_object(getThis());

    result = swf_object_replace_pngdata(swf, image_id,
                                        (unsigned char *)data,
                                        (unsigned long) data_len,
                                        rgb15);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
#endif /* HAVE_PNG */
}

PHP_METHOD(SWFEditor, replaceGIFData) {
#ifndef HAVE_GIF
    fprintf(stderr, "replaceGIFData: no gif library\n");
    RETURN_FALSE;
#else /* HAVE_GIF */
    zend_long image_id = 0;
    char *data = NULL;
    size_t data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &image_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_gifdata(swf, image_id,
                                        (unsigned char *)data,
                                        (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
#endif /* HAVE_GIF */
}

PHP_METHOD(SWFEditor, replaceBitmapData) {
    char *data = NULL, *alpha_data = NULL;
    size_t data_len = 0 , alpha_data_len = 0;
    zval *arg1 = NULL, *arg4 = NULL;
    int image_id = 0;
    HashTable *image_info_table = NULL;
    int width = -1, height = -1;
    int red = -1, green = -1, blue = -1;
    HashTable *opts_table = NULL;
    int without_converting = 0;
    int rgb15 = -1;
    swf_object_t *swf = NULL;
    int result = 0;
    int bitmap_format;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE; /* XXX */
    }
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
      case 3:
      case 4:
      case 5:
        if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET,
                                     ZEND_NUM_ARGS(), "zs|sz", &arg1, &data, &data_len, &alpha_data, &alpha_data_len, &arg4) == SUCCESS) {
            ; // OK
        } else {
            php_error(E_WARNING, "%s() expects parameter 2 to be string given", get_active_function_name());
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());

    if (Z_TYPE_P(arg1) != IS_ARRAY) { // image_id (integer)
        if (Z_TYPE_P(arg1) != IS_LONG) {
            convert_to_long(arg1);
        }
        image_id = (int) Z_LVAL_P(arg1);
    } else { // or image_info (array)
        image_info_table = Z_ARRVAL_P(arg1);
        get_zend_hash_value_long(image_info_table, "width",  width);
        get_zend_hash_value_long(image_info_table, "height", height);
        get_zend_hash_value_long(image_info_table, "red",    red);
        get_zend_hash_value_long(image_info_table, "green",  green);
        get_zend_hash_value_long(image_info_table, "blue",   blue);
        image_id = swf_object_search_cid_by_bitmap_condition(swf, width, height,
                                                             red, green, blue);
        if (image_id <= 0) {
            php_error(E_WARNING, "%s() cid not found by bitmap condition(width=%d height=%d red=%d green=%d blue=%d)", get_active_function_name(),
                      width, height, red, green, blue);
            RETURN_FALSE;
        }
    }
    if (arg4 != NULL) {
        if (Z_TYPE_P(arg4) != IS_ARRAY) { // without_converting (boolean)
            if (Z_TYPE_P(arg4) != IS_TRUE || Z_TYPE_P(arg4) != IS_FALSE) {
                convert_to_boolean(arg4);
            }
            without_converting = (int) Z_LVAL_P(arg4);
        } else { // or opts (array)
            opts_table = Z_ARRVAL_P(arg4);
            get_zend_hash_value_boolean(opts_table, "without_converting",  without_converting);
            get_zend_hash_value_boolean(opts_table, "rgb15", rgb15);
        }
    }
    bitmap_format = detect_bitmap_format((unsigned char*) data, data_len);
    if (without_converting) { // for v8 JPEG Tag
        switch (bitmap_format) {
        case BITMAP_UTIL_FORMAT_JPEG:
        case BITMAP_UTIL_FORMAT_PNG:
        case BITMAP_UTIL_FORMAT_GIF:
            result = swf_object_replace_jpegdata(swf, image_id,
                                                 (unsigned char *)data,
                                                 (unsigned long) data_len,
                                                 (unsigned char *) 0,
                                                 (unsigned long) 0,
                                                 1);
            break;
        default:
            php_error_docref(NULL, E_WARNING, "Unknown Bitmap Format");
            RETURN_FALSE;
        }
    } else {
        switch (bitmap_format) {
        case BITMAP_UTIL_FORMAT_JPEG:
            result = swf_object_replace_jpegdata(swf, image_id,
                                                 (unsigned char *)data,
                                                 (unsigned long) data_len,
                                                 (unsigned char *)alpha_data,
                                                 (unsigned long) alpha_data_len,
                                                 0);
            break;
        case BITMAP_UTIL_FORMAT_PNG:
            result = swf_object_replace_pngdata(swf, image_id,
                                                (unsigned char *)data,
                                                (unsigned long) data_len,
                                                rgb15);

            break;
        case BITMAP_UTIL_FORMAT_GIF:
            result = swf_object_replace_gifdata(swf, image_id,
                                                (unsigned char *)data,
                                                (unsigned long) data_len);

            break;
        default:
            php_error_docref(NULL, E_WARNING, "Unknown Bitmap Format");
            RETURN_FALSE;
        }
    }
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, convertBitmapDataToJpegTag) {
    swf_object_t *swf = NULL;
    int ret;
    swf = get_swf_object(getThis());
    ret = swf_object_convert_bitmapdata_tojpegtag(swf);
    if (ret) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, applyShapeMatrixFactor) {
    zend_long shape_id = 0;
    double scale_x = 1, scale_y = 1, rotate_rad = 0;
    zend_long trans_x = 0, trans_y = 0;
    swf_object_t *swf = NULL;
    int result;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE; /* XXX */
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "ldddll", &shape_id, &scale_x, &scale_y,
                              &rotate_rad, &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    result = swf_object_apply_shapematrix_factor(swf, shape_id, -1,
                                                 scale_x, scale_y, rotate_rad,
                                                 trans_x, trans_y);
    if (result) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, applyShapeRectFactor) {
    zend_long shape_id = 0;
    double scale_x = 1, scale_y = 1;
    zend_long trans_x = 0, trans_y = 0;
    swf_object_t *swf = NULL;
    int result;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE; /* XXX */
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "lddll", &shape_id, &scale_x, &scale_y,
                              &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    result = swf_object_apply_shaperect_factor(swf, shape_id, -1,
                                                 scale_x, scale_y,
                                                 trans_x, trans_y);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

/*
PHP_METHOD(SWFEditor, adjustShapeSizeToBitmap) {
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, adjustShapeScaleToBitmap) {
    RETURN_TRUE;
}
*/

PHP_METHOD(SWFEditor, getSoundData) {
    zend_long sound_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    swf_object_t *swf = NULL;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE; /* XXX */
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &sound_id) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
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
    //RETURN_STRINGL(new_buff, (int) len, 0);

    zend_string *str = zend_string_init(new_buff, len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceMLDData) {
    zend_long sound_id = 0;
    char *data = NULL;
    size_t data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE;
    }
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ls", &sound_id, &data, &data_len) == FAILURE) {
            RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_melodata(swf, sound_id,
                                         (unsigned char *)data,
                                         (unsigned long) data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getEditString) {
    char *var_name = NULL;
    size_t var_name_len = 0;
    swf_object_t *swf = NULL;
    char *data = NULL, *new_buff = NULL;
    int str_len = 0;
    int error = 0;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE;
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
                              &var_name, &var_name_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data = swf_object_get_editstring(swf, var_name, var_name_len, &error);
    if (data == NULL) {
        if (error) {
            RETURN_FALSE;
        } else {
            RETURN_NULL();
        }
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
    //RETURN_STRINGL(new_buff, str_len, 0);

    zend_string *str = zend_string_init(new_buff, str_len, 0);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, replaceEditString) {
    char *var_name = NULL, *ini_text = NULL;
    size_t var_name_len = 0, ini_text_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE;
    }
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss",
                              &var_name, &var_name_len,
                              &ini_text, &ini_text_len) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_editstring(swf, var_name,  var_name_len,
                                           ini_text, ini_text_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, getActionData) {
    zend_long tag_seqno = 0;
    swf_object_t *swf = NULL;
    unsigned char *data = NULL;
    char *new_buff = NULL;
    unsigned long data_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    data = swf_object_get_actiondata(swf, &data_len, tag_seqno);
    if (data == NULL) {
        fprintf(stderr, "getActionData: Can't get_actiondata\n");
        RETURN_FALSE;
    }

    new_buff = emalloc(data_len);
    if (new_buff == NULL) {
        fprintf(stderr, "getActionData: Can't emalloc new_buff\n");
        free(data);
        RETURN_FALSE;
    }
    memcpy(new_buff, data, data_len);
    //RETURN_STRINGL(new_buff, data_len, 0);

    zend_string *str = zend_string_init(new_buff, data_len, 0);
    free(data);
    RETURN_STR(str);
}

PHP_METHOD(SWFEditor, disasmActionData) {
    char *data = NULL;
    size_t data_len = 0;
    bitstream_t *bs = NULL;
    swf_action_list_t *action_list = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    array_init(return_value);
    bs = bitstream_open();
    bitstream_input(bs, (unsigned char*) data, data_len);
    action_list = swf_action_list_create();
    swf_action_list_parse(bs, action_list);
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

PHP_METHOD(SWFEditor, setActionVariables) {
    zval *zid, *arr, **entry;
    HashTable *arr_hash;
    char            *str_key, *str_value;
    uint            str_key_len, str_value_len;
    int ret;
    y_keyvalue_t *kv;
    swf_object_t *swf = get_swf_object(getThis());
    zend_ulong loop_num_key;
    zend_string *loop_key = NULL;
    zval *loop_val;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE) {
        RETURN_FALSE;
    }
    kv = y_keyvalue_open();

    arr_hash = Z_ARRVAL_P(arr);
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), loop_num_key, loop_key, loop_val) {
        if (! loop_key) {
            loop_key = zend_long_to_str(loop_num_key);
        }
        convert_to_string_ex(loop_val);
        str_key       = ZSTR_VAL(loop_key);
        str_key_len   = ZSTR_LEN(loop_key);
        str_value     = Z_STRVAL_P(loop_val);
        str_value_len = Z_STRLEN_P(loop_val);
	y_keyvalue_set(kv, str_key, str_key_len, str_value, str_value_len);
    } ZEND_HASH_FOREACH_END();
    swf_object_insert_action_setvariables(swf, kv);
    y_keyvalue_close(kv);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, replaceActionStrings) {
    zval *zid, *arr, **entry;
    HashTable *arr_hash;
    char            *str_key, *str_value;
    uint            str_key_len, str_value_len;
    int ret;
    y_keyvalue_t *kv;
    swf_object_t *swf = get_swf_object(getThis());
    zend_string *loop_key = NULL;
    zval *loop_val;
    zend_ulong loop_num_key;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE) {
        RETURN_FALSE;
    }
    kv = y_keyvalue_open();

    arr_hash = Z_ARRVAL_P(arr);
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), loop_num_key, loop_key, loop_val) {
        if (! loop_key) {
            loop_key = zend_long_to_str(loop_num_key);
        }
        convert_to_string_ex(loop_val);
        str_key       = ZSTR_VAL(loop_key);
        str_key_len   = ZSTR_LEN(loop_key);
        str_value     = Z_STRVAL_P(loop_val);
        str_value_len = Z_STRLEN_P(loop_val);
        y_keyvalue_set(kv, str_key, str_key_len, str_value, str_value_len);
    } ZEND_HASH_FOREACH_END();

    swf_object_replace_action_strings(swf, kv);
    y_keyvalue_close(kv);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, replaceMovieClip) {
    char *instance_name = NULL, *swf_data = NULL;
    size_t instance_name_len = 0, swf_data_len = 0;
    swf_object_t *swf = NULL;
    zend_bool unused_cid_purge = 1; // デフォルト on
    int result = 0;
    if (param_is_null(1)) {
        php_error(E_WARNING, "%s() 1st arg must be not NULL", get_active_function_name());
        RETURN_FALSE;
    }
    switch (ZEND_NUM_ARGS()) {
      default:
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
      case 2:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss",
                                  &instance_name, &instance_name_len,
                                  &swf_data, &swf_data_len) == FAILURE) {
          RETURN_FALSE;
        }
      break;
      case 3:
        if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssb",
                                  &instance_name, &instance_name_len,
                                  &swf_data, &swf_data_len, &unused_cid_purge) == FAILURE) {
            // unused_cid_purge は無視します。
          RETURN_FALSE;
        }
        break;
    }
    swf = get_swf_object(getThis());
    result = swf_object_replace_movieclip(swf,
                                          (unsigned char *)instance_name,
                                          instance_name_len,
                                          (unsigned char *)swf_data,
                                          swf_data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, rebuild) {
    swf_object_t *swf = get_swf_object(getThis());
    if (swf_object_rebuild(swf)) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, purgeUselessContents) {
    swf_object_t *swf = get_swf_object(getThis());
    swf_object_purge_contents(swf);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, setCompressLevel) {
    zend_long compress_level = 6 ; // Z_DEFAULT_COMPRESSION
    swf_object_t *swf;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &compress_level) == FAILURE) {
        RETURN_FALSE;
    }
    swf = get_swf_object(getThis());
    swf->compress_level = compress_level;
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, isShapeTagData) {
    char *data = NULL;
    size_t data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    if (swf_object_is_shape_tagdata((unsigned char *)data,
                                    data_len) == 0) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, isBitmapTagData) {
    char *data = NULL;
    size_t data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "s", &data, &data_len) == FAILURE) {
        RETURN_FALSE;
    }
    if (swf_object_is_bitmap_tagdata((unsigned char *)data,
                                     data_len) == 0) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}


static swf_object_t  *get_swf_object(zval *obj) {
//    zval *data, **tmp;
    zval *tmp = NULL;
    swf_object_t *swf = NULL;
    int id = 0, type = 0;
/* XXX: zend_read_property
    data = zend_read_property(Z_OBJCE_P(obj), obj, "swf_object",
                                                          strlen("swf_object"), 1);
*/
    if ((tmp = zend_hash_str_find(Z_OBJPROP_P(obj), ZEND_STRL("swf_object"))) == NULL) {
        printf("coudnt found swf_object\n");
        return NULL;
    }
    if (Z_RES_P(tmp)->type != le_swfed) {
        printf("invalid type[%d:%d]\n", Z_RES_P(tmp)->type, le_swfed);
        return NULL;
    }
    //id = Z_LVAL_P(tmp);
    //swf = (swf_object_t *) zend_list_find(id, &type);
    swf = (swf_object_t *) Z_RES_P(tmp)->ptr;
    return swf;
}

static void free_swfed_resource(zend_resource *resource)
{
//    printf("SWFEditor->destory\n");
    swf_object_close((swf_object_t *) resource->ptr);
}

