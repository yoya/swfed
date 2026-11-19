/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
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

#ifndef PHP_SWFED_H
#define PHP_SWFED_H

extern zend_module_entry swfed_module_entry;
#define phpext_swfed_ptr &swfed_module_entry

#define PHP_SWFED_VERSION "2.1" /* Replace with version number for your extention */

#ifdef PHP_WIN32
#define PHP_SWFED_API __declspec(dllexport)
#else
#define PHP_SWFED_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(swfed);
PHP_MSHUTDOWN_FUNCTION(swfed);
PHP_RINIT_FUNCTION(swfed);
PHP_RSHUTDOWN_FUNCTION(swfed);
PHP_MINFO_FUNCTION(swfed);

PHP_FUNCTION(confirm_swfed_compiled);	/* For testing, remove later. */

static zend_class_entry *swfeditor_ce;

PHP_METHOD(swfed, __construct);
PHP_METHOD(swfed, input);
PHP_METHOD(swfed, output);
PHP_METHOD(swfed, swfInfo);
PHP_METHOD(swfed, _destroy_and_exit); // for debug
//
PHP_METHOD(swfed, getHeaderInfo);
PHP_METHOD(swfed, setHeaderInfo);
PHP_METHOD(swfed, getTagList);
PHP_METHOD(swfed, getTagDetail);
PHP_METHOD(swfed, getTagInfo);
PHP_METHOD(swfed, getTagData);
PHP_METHOD(swfed, replaceTagData);
PHP_METHOD(swfed, getTagDataByCID);
PHP_METHOD(swfed, replaceTagDataByCID);
PHP_METHOD(swfed, getTagContentsByCID);
PHP_METHOD(swfed, replaceTagContentsByCID);
PHP_METHOD(swfed, removeTag);
PHP_METHOD(swfed, printTagData);
//
PHP_METHOD(swfed, getShapeData);
PHP_METHOD(swfed, replaceShapeData);
PHP_METHOD(swfed, setShapeAdjustMode);
PHP_METHOD(swfed, getShapeIdListByBitmapRef);
PHP_METHOD(swfed, getBitmapSize);
//
PHP_METHOD(swfed, getJpegData);
PHP_METHOD(swfed, getJpegAlpha);
PHP_METHOD(swfed, replaceJpegData);
PHP_METHOD(swfed, getPNGData);
PHP_METHOD(swfed, replacePNGData);
PHP_METHOD(swfed, replaceGIFData);
PHP_METHOD(swfed, replaceBitmapData);
PHP_METHOD(swfed, convertBitmapDataToJpegTag);
PHP_METHOD(swfed, applyShapeMatrixFactor);
PHP_METHOD(swfed, applyShapeRectFactor);
PHP_METHOD(swfed, getSoundData);
PHP_METHOD(swfed, replaceMLDData);
PHP_METHOD(swfed, getEditString);
PHP_METHOD(swfed, replaceEditString);
PHP_METHOD(swfed, getActionData);
PHP_METHOD(swfed, disasmActionData);
PHP_METHOD(swfed, setActionVariables);
PHP_METHOD(swfed, replaceActionStrings);
PHP_METHOD(swfed, replaceMovieClip);

PHP_METHOD(swfed, setCompressLevel);
PHP_METHOD(swfed, rebuild);
PHP_METHOD(swfed, purgeUselessContents);

PHP_METHOD(swfed, isShapeTagData);
PHP_METHOD(swfed, isBitmapTagData);

ZEND_BEGIN_MODULE_GLOBALS(swfed)
    char *error_output; // swfed.error_output: "stderr", "warning" or "none"
    char *last_error;   // malloc'ed, SWFEditor::getLastError()
ZEND_END_MODULE_GLOBALS(swfed)

#define SWFED_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(swfed, v)

#define REGISTER_SWFED_CLASS_CONST_LONG(const_name, value) \
    zend_declare_class_constant_long(swfed_entry_ptr, const_name, sizeof(const_name)-1, (long)value);


#endif	/* PHP_SWFED_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
