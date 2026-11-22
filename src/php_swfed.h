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

#define PHP_SWFED_VERSION "2.3" /* Replace with version number for your extention */

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

/* swfed.error_output */
#define SWFED_ERROR_OUTPUT_STDERR  0
#define SWFED_ERROR_OUTPUT_WARNING 1
#define SWFED_ERROR_OUTPUT_NONE    2

/* output of the C library is not passed to PHP while the library runs
 * (the user code it could run would change the data the library is walking),
 * it is kept in pending and written out after the method returns. */
#define SWFED_PENDING_PRINT   0 /* swfInfo() etc.: PHPWRITE() */
#define SWFED_PENDING_WARNING 1 /* swfed.error_output=warning: E_WARNING */
#define SWFED_PENDING_STDERR  2 /* swfed.error_output=stderr: kept in order with the others */
typedef struct swfed_pending_segment_ {
    int kind;
    size_t length; /* the bytes are in pending, in order */
} swfed_pending_segment;

ZEND_BEGIN_MODULE_GLOBALS(swfed)
    int error_output;   // swfed.error_output: SWFED_ERROR_OUTPUT_*
    char *last_error;   // malloc'ed, SWFEditor::getLastError()
    size_t last_error_len;
    size_t last_error_cap;
    char *pending;      // malloc'ed, output waiting for the library to return
    size_t pending_len;
    size_t pending_cap;
    swfed_pending_segment *pending_segments; // malloc'ed
    size_t pending_segments_num;
    size_t pending_segments_cap;
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
