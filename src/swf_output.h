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
/* the same format checking (-Wformat) as printf() / fprintf() */
#if defined(__GNUC__) || defined(__clang__)
#define SWF_PRINTF_FORMAT(format_idx, first_arg) \
    __attribute__((format(printf, format_idx, first_arg)))
#else
#define SWF_PRINTF_FORMAT(format_idx, first_arg)
#endif
/*
 * The upper limit (bytes) of what one decompression or one image conversion
 * may allocate: the data of DefineBitsLossless / the alpha of DefineBitsJPEG3,
 * a PNG or a GIF turned into a bitmap. The handler returns it, 0 for no limit.
 * Without a handler there is no limit.
 */
typedef unsigned long (*swf_size_limit_handler_t)(void);
extern void swf_set_size_limit_handler(swf_size_limit_handler_t handler);
extern unsigned long swf_size_limit(void);
/* returns 1 (with an error message that starts with func) when size is over the limit */
extern int swf_size_limit_exceeded(const char *func, unsigned long size);

extern int swf_printf(const char *format, ...) SWF_PRINTF_FORMAT(1, 2);
extern int swf_fprintf(FILE *fp, const char *format, ...) SWF_PRINTF_FORMAT(2, 3);

#endif /* __SWF_OUTPUT__H__ */
