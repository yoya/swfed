/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_MATRIX_H__
#define __SWF_MATRIX_H__

#include "swf_matrix.h"

typedef struct swf_matrix_ {
    // char align;
    unsigned char has_scale:1;
    unsigned char scale_bits; //  : 5;
    signed long scale_x; // : scale_bits;
    signed long scale_y; // : scale_bits;
} swf_matrix_t;

extern int swf_matrix_parse(bitstream_t *bs, swf_matrix_t *color);
extern int swf_matrix_build(bitstream_t *bs, swf_matrix_t *color);
extern int swf_matrix_print(swf_matrix_t *color);

#endif /* __SWF_MATRIX_H__ */
