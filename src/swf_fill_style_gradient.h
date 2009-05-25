/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_FILL_STYLE_GRADIENT_H__
#define __SWF_FILL_STYLE_GRADIENT_H__

#include "swf_matrix.h"
#include "swf_gradient.h"

typedef union swf_fill_style_gradient_ {
    unsigned char type;
    swf_matrix_t gradient_matrix;
    swf_matrix_t gradient_matrix_morph; // DefineMorphShape, DefineMorphShape2
    swf_gradient_t gradient;
} swf_fill_style_gradient_t;

extern int swf_fill_style_gradient_parse(bitstream_t *bs, swf_fill_style_gradient_t *color);
extern int swf_fill_style_gradient_build(bitstream_t *bs, swf_fill_style_gradient_t *color);
extern int swf_fill_style_gradient_print(swf_fill_style_gradient_t *color, int indent_depth);

#endif /* __SWF_FILL_STYLE_GRADIENT_H__ */
