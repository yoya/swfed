/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_OUTPUT__H__
#define __SWF_OUTPUT__H__

#include <stdio.h>

/*
 * The library writes its print output (printf) and error messages
 * (fprintf(stderr, ...)) through swf_printf() / swf_fprintf()
 * (swf_define.h maps printf / fprintf to them), so that an application
 * can catch them with swf_set_output_handler().
 * Without a handler they go to stdout / stderr.
 */
typedef void (*swf_output_handler_t)(const char *message, size_t length);

extern void swf_set_output_handler(swf_output_handler_t print_handler,
                                   swf_output_handler_t error_handler);
extern int swf_printf(const char *format, ...);
extern int swf_fprintf(FILE *fp, const char *format, ...);

#endif /* __SWF_OUTPUT__H__ */
