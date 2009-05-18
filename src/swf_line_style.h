/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_LINE_STYLE_H__
#define __SWF_LINE_STYLE_H__

#include "swf_rgba.h"

typedef union swf_line_style_ {
    // DefineMorphShape
    unsigned short width; // twips
    unsigned short width_morph; // twips
    swf_rgba_t rgba;
    swf_rgba_t rgba_morph;
    // DefineShape4, DefinePhophShape2
//    ...
} swf_line_style_t;

extern int swf_line_style_parse(bitstream_t *bs, swf_line_style_t *color);
extern int swf_line_style_build(bitstream_t *bs, swf_line_style_t *color);
extern int swf_line_style_print(swf_line_style_t *color);

#endif /* __SWF_LINE_STYLE_H__ */
