/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_FILL_STYLE_ARRAY_H__
#define __SWF_FILL_STYLE_ARRAY_H__

#include "swf_tag.h"
#include "swf_fill_style.h"

typedef struct swf_fill_style_array_ {
    unsigned short count;
    swf_fill_style_t *fill_style;
} swf_fill_style_array_t;

extern int swf_fill_style_array_parse(bitstream_t *bs, swf_fill_style_array_t *color, swf_tag_t *tag);
extern int swf_fill_style_array_build(bitstream_t *bs, swf_fill_style_array_t *color);
extern int swf_fill_style_array_print(swf_fill_style_array_t *color, int indent_depth);

#endif /* __SWF_FILL_STYLE_ARRAY_H__ */
