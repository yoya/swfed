/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_FILL_STYLE_BITMAP_H__
#define __SWF_FILL_STYLE_BITMAP_H__

#include "swf_matrix.h"

typedef struct swf_fill_style_bitmap_ {
    unsigned char type;
    unsigned short bitmap_ref;
    swf_matrix_t bitmap_matrix;
    swf_matrix_t bitmap_matrix_morph; // DefineMorphShape, DefineMorphShape2
} swf_fill_style_bitmap_t;

extern int swf_fill_style_bitmap_parse(bitstream_t *bs, swf_fill_style_bitmap_t *color);
extern int swf_fill_style_bitmap_build(bitstream_t *bs, swf_fill_style_bitmap_t *color);
extern int swf_fill_style_bitmap_print(swf_fill_style_bitmap_t *color, int indent_depth);

#endif /* __SWF_FILL_STYLE_BITMAP_H__ */
