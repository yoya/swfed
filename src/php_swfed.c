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

#include <limits.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_interfaces.h"
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
            value = zval_get_long(tmp); \
        } \
    } while (0);

#define get_zend_hash_value_boolean(table, key, value) do { \
        zval *tmp = NULL; \
        if ((tmp = zend_hash_str_find(table, ZEND_STRL(key))) != NULL) { \
            value = zend_is_true(tmp); \
        } \
    } while (0);

static zend_class_entry *swfed_entry_ptr = NULL;


/* If you declare any globals in php_swfed.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(swfed)
*/

/* SWFEditor keeps its swf_object_t in the object itself (not in a property),
 * so it is freed with the object and invisible to foreach / serialize. */
typedef struct swfed_object_ {
    swf_object_t *swf;
    int constructed;
    zend_object std;
} swfed_object;

static zend_object_handlers swfed_object_handlers;

static inline swfed_object *swfed_from_obj(zend_object *obj) {
    return (swfed_object *)((char *)obj - XtOffsetOf(swfed_object, std));
}

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
/* swfed.max_uncompressed_size: upper limit of the SWF size after decompression
 * (the header file_length of a compressed SWF: CWS / ZWS). The C library
 * allocates with malloc(), outside memory_limit.
 * 0 (default) uses memory_limit, -1 means unlimited. */
PHP_INI_BEGIN()
    PHP_INI_ENTRY("swfed.max_uncompressed_size", "0", PHP_INI_ALL, NULL)
PHP_INI_END()
/* }}} */

static zend_long swfed_parse_size(const char *value) {
#if PHP_VERSION_ID >= 80200
    zend_string *str = zend_string_init(value, strlen(value), 0);
    zend_string *errstr = NULL;
    zend_long size = zend_ini_parse_quantity(str, &errstr);
    zend_string_release(str);
    if (errstr) {
        zend_string_release(errstr);
    }
    return size;
#else
    return zend_atol(value, strlen(value));
#endif
}

/* returns -1 for unlimited */
static zend_long swfed_max_uncompressed_size(void) {
    const char *value = INI_STR("swfed.max_uncompressed_size");
    zend_long limit = (value && value[0]) ? swfed_parse_size(value) : 0;
    if (limit == 0) {
        if ((PG(memory_limit) == 0) ||
            (PG(memory_limit) >= (size_t) ZEND_LONG_MAX)) {
            return -1;
        }
        return (zend_long) PG(memory_limit);
    }
    return (limit < 0) ? -1 : limit;
}

/* checks the file_length in the header of a compressed SWF (CWS / ZWS), which
 * is the size after decompression; warns and returns 1 if too large.
 * An uncompressed SWF (FWS) needs no decompression, so its memory use is
 * proportional to the input, and non SWF data is rejected by the C library. */
static int swfed_check_uncompressed_size(const char *data, size_t data_len) {
    zend_long limit;
    unsigned long file_length;
    if ((data_len < 8) ||
        ((memcmp(data, "CWS", 3) != 0) && (memcmp(data, "ZWS", 3) != 0))) {
        return 0;
    }
    limit = swfed_max_uncompressed_size();
    if (limit < 0) {
        return 0;
    }
    file_length = (unsigned long) (unsigned char) data[4] |
                  ((unsigned long) (unsigned char) data[5] << 8) |
                  ((unsigned long) (unsigned char) data[6] << 16) |
                  ((unsigned long) (unsigned char) data[7] << 24);
    if (file_length > (zend_ulong) limit) {
        php_error_docref(NULL, E_WARNING,
                         "SWF file_length(%lu) exceeds the limit(" ZEND_LONG_FMT
                         ") of swfed.max_uncompressed_size", file_length, limit);
        return 1;
    }
    return 0;
}

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


static zend_object *swfed_create_object(zend_class_entry *ce);
static void swfed_free_object(zend_object *obj);
#if PHP_VERSION_ID >= 80000
static zend_object *swfed_clone_object(zend_object *old_obj);
#else
static zend_object *swfed_clone_object(zval *old_zv);
#endif
static swf_object_t  *get_swf_object(zval *obj);

#ifndef RETURN_THROWS
#define RETURN_THROWS() return
#endif
/* character ids are 16bit in SWF; tag positions are passed on as int */
#define SWFED_CHECK_CID(v) do { \
        if (((v) < 0) || ((v) > 65535)) { \
            php_error_docref(NULL, E_WARNING, #v "(" ZEND_LONG_FMT \
                             ") must be between 0 and 65535", (zend_long) (v)); \
            RETURN_FALSE; \
        } \
    } while (0)
#define SWFED_CHECK_INDEX(v, min) do { \
        if (((v) < (min)) || ((v) > INT_MAX)) { \
            php_error_docref(NULL, E_WARNING, #v "(" ZEND_LONG_FMT \
                             ") must be between %d and %d", (zend_long) (v), \
                             (min), INT_MAX); \
            RETURN_FALSE; \
        } \
    } while (0)
/* RECT is written with 5bit NBits, i.e. up to 31bit signed twips */
#define SWFED_RECT_TWIPS_MIN (-1073741824.0)
#define SWFED_RECT_TWIPS_MAX (1073741823.0)

/* get_swf_object() has thrown an Error when it returns NULL */
#define SWFED_GET_OBJECT(swf) do { \
        (swf) = get_swf_object(getThis()); \
        if ((swf) == NULL) { \
            RETURN_THROWS(); \
        } \
    } while (0)

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(swfed)
{
    REGISTER_INI_ENTRIES();
    swfed_entry_ptr = register_class_SWFEditor();
    swfed_entry_ptr->create_object = swfed_create_object;
    memcpy(&swfed_object_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));
    swfed_object_handlers.offset = XtOffsetOf(swfed_object, std);
    swfed_object_handlers.free_obj = swfed_free_object;
    swfed_object_handlers.clone_obj = swfed_clone_object;
    #if ZEND_MODULE_API_NO >= 20220829 // php 8.2
    swfed_entry_ptr->ce_flags |= ZEND_ACC_ALLOW_DYNAMIC_PROPERTIES;
    #endif
    /* the SWF data lives outside the properties, so serialize would lose it */
    #if PHP_VERSION_ID >= 80100
    swfed_entry_ptr->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
    #else
    swfed_entry_ptr->serialize = zend_class_serialize_deny;
    swfed_entry_ptr->unserialize = zend_class_unserialize_deny;
    #endif
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
        UNREGISTER_INI_ENTRIES();
        return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(swfed)
{
#if defined(COMPILE_DL_SWFED) && defined(ZTS)
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

        DISPLAY_INI_ENTRIES();
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

PHP_METHOD(SWFEditor, __construct) {
    swfed_object *intern = swfed_from_obj(Z_OBJ_P(getThis()));
    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_THROWS();
    }
    /* re-initializing would free the swf_object_t a running method may hold */
    if (intern->constructed) {
        zend_throw_error(NULL, "SWFEditor object is already initialized");
        RETURN_THROWS();
    }
    intern->constructed = 1;
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
    SWFED_GET_OBJECT(swf);
    if (swfed_check_uncompressed_size(data, data_len)) {
        RETURN_FALSE;
    }
    result = swf_object_input(swf, (unsigned char *) data, data_len);
    if (result) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, output) {
    unsigned long len = 0;
    unsigned char *data = NULL;
    swf_object_t *swf = NULL;
    if (ZEND_NUM_ARGS() != 0) {
        WRONG_PARAM_COUNT;
        RETURN_FALSE; /* XXX */
    }
    SWFED_GET_OBJECT(swf);
    data = swf_object_output(swf, &len);
    if (data == NULL) {
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, len);
    free(data);
}

PHP_METHOD(SWFEditor, swfInfo) {
    swf_object_t *swf = NULL;
    SWFED_GET_OBJECT(swf);
    swf_object_print(swf);
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, _destroy_and_exit) { // kept for API compat; must not exit() the worker
    RETURN_TRUE;
}

/* pixels as int when exact, otherwise float, so get/setHeaderInfo round-trips */
static void swfed_add_assoc_pixel(zval *arr, const char *key, int twips) {
    if (twips % SWF_TWIPS == 0) {
        add_assoc_long(arr, key, twips / SWF_TWIPS);
    } else {
        add_assoc_double(arr, key, (double) twips / SWF_TWIPS);
    }
}

PHP_METHOD(SWFEditor, getHeaderInfo) {
    swf_object_t *swf = NULL;
    SWFED_GET_OBJECT(swf);
    array_init(return_value);
    if (memcmp(swf->header.magic, "CWS", 3) == 0) {
        add_assoc_bool(return_value, "compress", 1);
    } else { // FWS
        add_assoc_bool(return_value, "compress", 0);
    }
    add_assoc_long(return_value, "version", swf->header.version);

    swfed_add_assoc_pixel(return_value, "x_min", swf->header_movie.frame_size.x_min);
    swfed_add_assoc_pixel(return_value, "y_min", swf->header_movie.frame_size.y_min);
    swfed_add_assoc_pixel(return_value, "x_max", swf->header_movie.frame_size.x_max);
    swfed_add_assoc_pixel(return_value, "y_max", swf->header_movie.frame_size.y_max);
}

PHP_METHOD(SWFEditor, setHeaderInfo) {
    static const char *rect_keys[4] = { "x_min", "y_min", "x_max", "y_max" };
    zval *header_info = NULL;
    swf_object_t *swf = NULL;
    HashTable *header_table = NULL;
    zval *tmp = NULL;
    zend_long version = -1;
    int i, has_rect[4] = { 0, 0, 0, 0 }, rect_twips[4] = { 0, 0, 0, 0 };
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a",
                              &header_info) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_GET_OBJECT(swf);
    header_table = Z_ARRVAL_P(header_info);

    /* validate everything before changing the header */
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("version"))) != NULL) {
        version = zval_get_long(tmp);
        if ((version < 0) || (version > 255)) {
            php_error_docref(NULL, E_WARNING, "version(" ZEND_LONG_FMT
                             ") must be between 0 and 255", version);
            RETURN_FALSE;
        }
    }
    for (i = 0 ; i < 4 ; i++) {
        double twips;
        tmp = zend_hash_str_find(header_table, rect_keys[i], strlen(rect_keys[i]));
        if (tmp == NULL) {
            continue;
        }
        twips = zval_get_double(tmp) * SWF_TWIPS;
        if (! ((twips >= SWFED_RECT_TWIPS_MIN) && (twips <= SWFED_RECT_TWIPS_MAX))) {
            php_error_docref(NULL, E_WARNING, "%s(%.17g) must be between %.2f and %.2f",
                             rect_keys[i], zval_get_double(tmp),
                             SWFED_RECT_TWIPS_MIN / SWF_TWIPS,
                             SWFED_RECT_TWIPS_MAX / SWF_TWIPS);
            RETURN_FALSE;
        }
        has_rect[i] = 1;
        rect_twips[i] = (int) (twips < 0 ? twips - 0.5 : twips + 0.5);
    }

    // FWS or CWS
    if ((tmp = zend_hash_str_find(header_table, ZEND_STRL("compress"))) != NULL) {
        if (zend_is_true(tmp)) {
            memcpy(swf->header.magic, "CWS", 3);
        } else {
            memcpy(swf->header.magic, "FWS", 3);
        }
    }
    if (version >= 0) {
        swf->header.version = (unsigned char) version;
    }
    // FrameRect
    if (has_rect[0]) { swf->header_movie.frame_size.x_min = rect_twips[0]; }
    if (has_rect[1]) { swf->header_movie.frame_size.y_min = rect_twips[1]; }
    if (has_rect[2]) { swf->header_movie.frame_size.x_max = rect_twips[2]; }
    if (has_rect[3]) { swf->header_movie.frame_size.y_max = rect_twips[3]; }
    RETURN_TRUE;
}

/* the tags getTagDetail() can describe; keep in sync with its switch */
static int swfed_tag_detail_supported(int code) {
    switch (code) {
      case 2: case 4: case 6: case 12: case 14: case 20: case 21: case 22:
      case 26: case 32: case 35: case 36: case 37: case 39: case 46: case 59:
      case 83: case 84:
      case 90:
        return 1;
    }
    return 0;
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
    SWFED_GET_OBJECT(swf);
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
        add_assoc_long(&data, "length", swf_tag_get_length(tag, swf));
        if (tag_info && tag_info->detail_handler &&
            swfed_tag_detail_supported(tag->code)) {
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
    SWFED_CHECK_INDEX(tag_seqno, 0);
    SWFED_GET_OBJECT(swf);
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
    if ((tag_info == NULL) || (tag_info->detail_handler == NULL) ||
        (! swfed_tag_detail_supported(tag->code))) {
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
      case 90: // DefineBitsJPEG4
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
                                sizeof("variable_name") - 1,
                                (char *)tag_edit->edit_variable_name);
        }
        if (tag_edit->edit_initial_text && tag_edit->edit_initial_text[0]) {
            add_assoc_string_ex(return_value, "initial_text",
                                sizeof("initial_text") - 1,
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
      case 83: // DefineShape4;
      case 84: // DefineMorphShape2;
        tag_shape = tag->detail;
        array_init(return_value);
        add_assoc_long(return_value, "shape_id", tag_shape->shape_id);
        if (tag_shape->is_morph) { // DefineMorphShape / DefineMorphShape2
            /* They keep the style arrays in morph_shape_with_style;
             * shape_with_style is left empty. */
            add_assoc_long(return_value, "fill_styles.count",
                           tag_shape->morph_shape_with_style.styles.fill_styles.count);
            add_assoc_long(return_value, "line_styles.count",
                           tag_shape->morph_shape_with_style.styles.line_styles.count);
        } else {
            add_assoc_long(return_value, "fill_styles.count", tag_shape->shape_with_style.styles.fill_styles.count);
            add_assoc_long(return_value, "line_styles.count", tag_shape->shape_with_style.styles.line_styles.count);
        }
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
                                sizeof("name") - 1, (char *)tag_place->name);
        }
        break;
      default:
        RETURN_FALSE;
    }
}

PHP_METHOD(SWFEditor, getTagData) {
    zend_long tag_seqno = 0;
    swf_object_t *swf = NULL;
    unsigned char *data = NULL;
    unsigned long data_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_INDEX(tag_seqno, 0);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_tagdata(swf, tag_seqno, &data_len);

    if (data == NULL) {
        fprintf(stderr, "getTagData: Can't get_tagdata\n");
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, data_len);
    free(data);
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
    SWFED_CHECK_INDEX(tag_seqno, 0);
    SWFED_GET_OBJECT(swf);
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
    char *data = NULL;
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
    data = (char *) swf_object_get_tagdata_bycid(swf, cid, &data_len);

    if (data == NULL) {
        fprintf(stderr, "getTagDataByCID: Can't get_tagdata_bycid\n");
        RETURN_FALSE;
    }
    RETVAL_STRINGL(data, data_len);
    free(data);
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
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
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
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
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
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
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
    zend_bool index_is_null = 1;
    swf_object_t *swf = NULL;
    int ret;

    /* "l|l!" handles 1 or 2 args; 0 args or 3+ args raise ArgumentCountError.
     * null for the 2nd arg sets index_is_null so it behaves like omitted (-1). */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l!",
                              &tag_seqno,
                              &tag_seqno_in_sprite, &index_is_null) == FAILURE) {
        RETURN_FALSE;
    }
    if (index_is_null) {
        tag_seqno_in_sprite = -1;
    }
    SWFED_CHECK_INDEX(tag_seqno, 0);
    SWFED_CHECK_INDEX(tag_seqno_in_sprite, -1);
    SWFED_GET_OBJECT(swf);
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
    SWFED_GET_OBJECT(swf);
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
    unsigned long data_len = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &cid) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_shapedata(swf, cid, &data_len);

    if (data == NULL) {
        fprintf(stderr, "getShapeData: Can't swf_object_get_shapedata (cid=%ld)\n", (long)cid);
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, data_len);
    free(data);
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
    SWFED_CHECK_CID(cid);
    SWFED_GET_OBJECT(swf);
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
    SWFED_GET_OBJECT(swf);
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
    SWFED_CHECK_CID(bitmap_id);
    SWFED_GET_OBJECT(swf);
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
    SWFED_CHECK_CID(bitmap_id);
    SWFED_GET_OBJECT(swf);
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
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_jpegdata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, len);
    free(data);
}

PHP_METHOD(SWFEditor, getJpegAlpha) {
    zend_long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_alphadata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, len);
    free(data);
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
    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);
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
    fprintf(stderr, "getPNGData: no png library\n");
    RETURN_FALSE;
#else /* HAVE_PNG */
    zend_long image_id = 0;
    unsigned long len = 0;
    unsigned char *data = NULL;
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &image_id) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_pngdata(swf, &len, image_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, len);
    free(data);
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

    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);

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
    SWFED_CHECK_CID(image_id);
    SWFED_GET_OBJECT(swf);
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
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "zs|s!z", &arg1, &data, &data_len,
                              &alpha_data, &alpha_data_len, &arg4) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_GET_OBJECT(swf);

    if (Z_TYPE_P(arg1) != IS_ARRAY) { // image_id (integer)
        if (Z_TYPE_P(arg1) != IS_LONG) {
            convert_to_long(arg1);
        }
        SWFED_CHECK_CID(Z_LVAL_P(arg1));
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
            without_converting = zend_is_true(arg4);
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
#ifdef HAVE_PNG
            result = swf_object_replace_pngdata(swf, image_id,
                                                (unsigned char *)data,
                                                (unsigned long) data_len,
                                                rgb15);
#else
            fprintf(stderr, "replaceBitmapData: no png library\n");
            RETURN_FALSE;
#endif
            break;
        case BITMAP_UTIL_FORMAT_GIF:
#ifdef HAVE_GIF
            result = swf_object_replace_gifdata(swf, image_id,
                                                (unsigned char *)data,
                                                (unsigned long) data_len);
#else
            fprintf(stderr, "replaceBitmapData: no gif library\n");
            RETURN_FALSE;
#endif
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
    SWFED_GET_OBJECT(swf);
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
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "ldddll", &shape_id, &scale_x, &scale_y,
                              &rotate_rad, &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(shape_id);
    SWFED_GET_OBJECT(swf);
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
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "lddll", &shape_id, &scale_x, &scale_y,
                              &trans_x, &trans_y) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(shape_id);
    SWFED_GET_OBJECT(swf);
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
    swf_object_t *swf = NULL;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &sound_id) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_CID(sound_id);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_sounddata(swf, &len, sound_id);
    if (data == NULL) {
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, len);
    free(data);
}

PHP_METHOD(SWFEditor, replaceMLDData) {
    zend_long sound_id = 0;
    char *data = NULL;
    size_t data_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
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
    SWFED_CHECK_CID(sound_id);
    SWFED_GET_OBJECT(swf);
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
    char *data = NULL;
    int str_len = 0;
    int error = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
                              &var_name, &var_name_len) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_editstring(swf, var_name, var_name_len, &error);
    if (data == NULL) {
        if (error) {
            RETURN_FALSE;
        } else {
            RETURN_EMPTY_STRING(); // the field exists but has no initial text
        }
    }
    str_len = strlen(data);
    RETVAL_STRINGL(data, str_len);
    free(data);
}

PHP_METHOD(SWFEditor, replaceEditString) {
    char *var_name = NULL, *ini_text = NULL;
    size_t var_name_len = 0, ini_text_len = 0;
    swf_object_t *swf = NULL;
    int result = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss",
                              &var_name, &var_name_len,
                              &ini_text, &ini_text_len) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_GET_OBJECT(swf);
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
    unsigned long data_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &tag_seqno) == FAILURE) {
        RETURN_FALSE;
    }
    SWFED_CHECK_INDEX(tag_seqno, 0);
    SWFED_GET_OBJECT(swf);
    data = swf_object_get_actiondata(swf, &data_len, tag_seqno);
    if (data == NULL) {
        fprintf(stderr, "getActionData: Can't get_actiondata\n");
        RETURN_FALSE;
    }
    RETVAL_STRINGL((char *)data, data_len);
    free(data);
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
    bs = bitstream_open();
    bitstream_input(bs, (unsigned char*) data, data_len);
    action_list = swf_action_list_create();
    swf_action_list_parse(bs, action_list);
    bitstream_close(bs);
    if (action_list) {
        swf_action_t *action = action_list->head;
        while(action) {
            swf_action_print(action, 1);
            action = action->next;
        }
    }
    swf_action_list_destroy(action_list);
    RETURN_TRUE;
}

/* copy a PHP array into kv without modifying the (possibly shared/immutable) array */
static y_keyvalue_t *
_array_to_keyvalue(zval *arr) {
    y_keyvalue_t *kv = y_keyvalue_open();
    zend_ulong loop_num_key;
    zend_string *loop_key = NULL;
    zval *loop_val;
    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), loop_num_key, loop_key, loop_val) {
        zend_string *key = loop_key ? zend_string_copy(loop_key) : zend_long_to_str(loop_num_key);
        zend_string *value = zval_get_string(loop_val);
        y_keyvalue_set(kv, ZSTR_VAL(key), ZSTR_LEN(key), ZSTR_VAL(value), ZSTR_LEN(value));
        zend_string_release(value);
        zend_string_release(key);
    } ZEND_HASH_FOREACH_END();
    return kv;
}

PHP_METHOD(SWFEditor, setActionVariables) {
    zval *arr;
    y_keyvalue_t *kv;
    swf_object_t *swf = NULL;
    int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE) {
        RETURN_FALSE;
    }
    /* Convert the array into kv first so an uncatchable Error from
       zval_get_string() does not leave the SWF half modified. */
    kv = _array_to_keyvalue(arr);
    if (EG(exception)) {
        y_keyvalue_close(kv);
        RETURN_THROWS();
    }
    SWFED_GET_OBJECT(swf);
    ret = swf_object_insert_action_setvariables(swf, kv);
    y_keyvalue_close(kv);
    if (ret) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, replaceActionStrings) {
    zval *arr;
    y_keyvalue_t *kv;
    swf_object_t *swf = NULL;
    int ret;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &arr) == FAILURE) {
        RETURN_FALSE;
    }
    kv = _array_to_keyvalue(arr);
    if (EG(exception)) {
        y_keyvalue_close(kv);
        RETURN_THROWS();
    }
    SWFED_GET_OBJECT(swf);
    ret = swf_object_replace_action_strings(swf, kv);
    y_keyvalue_close(kv);
    if (ret) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, replaceMovieClip) {
    char *instance_name = NULL, *swf_data = NULL;
    size_t instance_name_len = 0, swf_data_len = 0;
    swf_object_t *swf = NULL;
    zend_bool unused_cid_purge = 1; // デフォルト on
    int result = 0;
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
    SWFED_GET_OBJECT(swf);
    if (swfed_check_uncompressed_size(swf_data, swf_data_len)) {
        RETURN_FALSE;
    }
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
    swf_object_t *swf = NULL;
    SWFED_GET_OBJECT(swf);
    if (swf_object_rebuild(swf)) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, purgeUselessContents) {
    swf_object_t *swf = NULL;
    SWFED_GET_OBJECT(swf);
    if (swf_object_purge_contents(swf)) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_METHOD(SWFEditor, setCompressLevel) {
    zend_long compress_level = 6 ; // Z_DEFAULT_COMPRESSION
    swf_object_t *swf;
    if (zend_parse_parameters(ZEND_NUM_ARGS(),
                              "l", &compress_level) == FAILURE) {
        RETURN_FALSE;
    }
    if (compress_level < -1 || compress_level > 9) {
        php_error_docref(NULL, E_WARNING,
                         "setCompressLevel: compress_level must be -1..9 (got %ld)",
                         (long)compress_level);
        RETURN_FALSE;
    }
    SWFED_GET_OBJECT(swf);
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
    swfed_object *intern;
    if ((obj == NULL) || (Z_TYPE_P(obj) != IS_OBJECT)) {
        zend_throw_error(NULL, "SWFEditor object is not initialized");
        return NULL;
    }
    intern = swfed_from_obj(Z_OBJ_P(obj));
    if (intern->swf == NULL) {
        zend_throw_error(NULL, "SWFEditor object is not initialized");
        return NULL;
    }
    return intern->swf;
}

static zend_object *swfed_create_object(zend_class_entry *ce) {
    swfed_object *intern = ecalloc(1, sizeof(swfed_object) +
                                   zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &swfed_object_handlers;
    /* allocated here (not in __construct) so the object is always usable,
     * e.g. from a subclass constructor that doesn't call the parent one */
    intern->swf = swf_object_open();
    return &intern->std;
}

static void swfed_free_object(zend_object *obj) {
    swfed_object *intern = swfed_from_obj(obj);
    if (intern->swf) {
        swf_object_close(intern->swf);
        intern->swf = NULL;
    }
    zend_object_std_dtor(obj);
}

#if PHP_VERSION_ID >= 80000
static zend_object *swfed_clone_object(zend_object *old_obj) {
#else
static zend_object *swfed_clone_object(zval *old_zv) {
    zend_object *old_obj = Z_OBJ_P(old_zv);
#endif
    swfed_object *old_intern = swfed_from_obj(old_obj);
    zend_object *new_obj = swfed_create_object(old_obj->ce);
    swfed_object *new_intern = swfed_from_obj(new_obj);
    swf_object_t *from = old_intern->swf, *to = new_intern->swf;

    zend_objects_clone_members(new_obj, old_obj);
    new_intern->constructed = old_intern->constructed;
    if (from && to) {
        if (from->tag_head) {
            /* deep copy through the serialized form */
            unsigned long len = 0;
            unsigned char *data = swf_object_output(from, &len);
            if ((data == NULL) || swf_object_input(to, data, len)) {
                zend_throw_error(NULL, "SWFEditor: failed to clone the SWF data");
            }
            free(data);
        }
        to->header = from->header;
        to->header_movie = from->header_movie;
        to->shape_adjust_mode = from->shape_adjust_mode;
        to->compress_level = from->compress_level;
    }
    return new_obj;
}

