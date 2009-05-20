/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_FILL_STYLE_SOLID_H__
#define __SWF_FILL_STYLE_SOLID_H__

#include "swf_rgba.h"
#include "swf_rgb.h"

typedef union swf_fill_style_solid_ {
    unsigned char type;
    swf_rgba_t rgba;       // DefineMorphShape, DefineMorphShape2, DefineShape3
    swf_rgba_t rgba_morph; // DefineMorphShape, DefineMorphShape2
    swf_rgb_t  rgb;        // other tag
} swf_fill_style_solid_t;

extern int swf_fill_style_solid_parse(bitstream_t *bs, swf_fill_style_solid_t *color);
extern int swf_fill_style_solid_build(bitstream_t *bs, swf_fill_style_solid_t *color);
extern int swf_fill_style_solid_print(swf_fill_style_solid_t *color);

#endif /* __SWF_FILL_STYLE_SOLID_H__ */
