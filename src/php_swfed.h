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

#ifndef PHP_SWFED_H
#define PHP_SWFED_H

extern zend_module_entry swfed_module_entry;
#define phpext_swfed_ptr &swfed_module_entry

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
//
PHP_METHOD(swfed, getHeaderInfo);
PHP_METHOD(swfed, setHeaderInfo);
PHP_METHOD(swfed, getTagList);
PHP_METHOD(swfed, getTagDetail);
PHP_METHOD(swfed, getTagInfo);
PHP_METHOD(swfed, getTagData);
PHP_METHOD(swfed, replaceTagData);
PHP_METHOD(swfed, getTagContentsByCID);
PHP_METHOD(swfed, replaceTagContentsByCID);
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
PHP_METHOD(swfed, applyShapeMatrixFactor);
PHP_METHOD(swfed, applyShapeRectFactor);
PHP_METHOD(swfed, getSoundData);
PHP_METHOD(swfed, replaceMLDData);
PHP_METHOD(swfed, getEditString);
PHP_METHOD(swfed, replaceEditString);
PHP_METHOD(swfed, getActionData);
PHP_METHOD(swfed, disasmActionData);
PHP_METHOD(swfed, setActionVariables);
PHP_METHOD(swfed, replaceMovieClip);
PHP_METHOD(swfed, swfInfo);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(swfed)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(swfed)
*/

/* In every utility function you add that needs to use variables 
   in php_swfed_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SWFED_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SWFED_G(v) TSRMG(swfed_globals_id, zend_swfed_globals *, v)
#else
#define SWFED_G(v) (swfed_globals.v)
#endif

#define REGISTER_SWFED_CLASS_CONST_LONG(const_name, value) \
    zend_declare_class_constant_long(swfeditor_ce, const_name, sizeof(const_name)-1, (long)value TSRMLS_CC);


#endif	/* PHP_SWFED_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
