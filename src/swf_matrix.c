#include <stdio.h>
#include <math.h>
#include "bitstream.h"
#include "swf_matrix.h"

int
swf_matrix_parse(bitstream_t *bs, swf_matrix_t *matrix) {
    int translate_bits;
    bitstream_align(bs);
    matrix->has_scale = bitstream_getbit(bs);
    if (matrix->has_scale) {
        int scale_bits = bitstream_getbits(bs, 5);
        matrix->scale_x = bitstream_getbits(bs, scale_bits);
        matrix->scale_y = bitstream_getbits(bs, scale_bits);
    }
    matrix->has_rotate = bitstream_getbit(bs);
    if (matrix->has_rotate) {
        int rotate_bits = bitstream_getbits(bs, 5);
        matrix->rotate_skew0 = bitstream_getbits(bs, rotate_bits);
        matrix->rotate_skew1 = bitstream_getbits(bs, rotate_bits);
    }
    translate_bits = bitstream_getbits(bs, 5);
    matrix->translate_x = bitstream_getbits(bs, translate_bits);
    matrix->translate_y = bitstream_getbits(bs, translate_bits);
    return 0;
}

int
swf_matrix_build(bitstream_t *bs, swf_matrix_t *matrix) {
    int translate_bits;
    float log2_x, log2_y;
    bitstream_align(bs);
    bitstream_putbit(bs, matrix->has_scale);
    if (matrix->has_scale) {
        int scale_bits;
        log2_x = log2f((float) matrix->scale_x);
        log2_y = log2f((float) matrix->scale_y);
        scale_bits = (log2_x > log2_y)? log2_x: log2_y;
        scale_bits += 1; // XXX
        bitstream_putbits(bs, scale_bits, 5);
        bitstream_putbits(bs, matrix->scale_x, scale_bits);
        bitstream_putbits(bs, matrix->scale_y, scale_bits);
    }
    bitstream_putbit(bs, matrix->has_rotate);
    if (matrix->has_rotate) {
        int rotate_bits;
        float log2_skew0 = log2f((float) matrix->rotate_skew0);
        float log2_skew1 = log2f((float) matrix->rotate_skew1);
        rotate_bits = (log2_skew0 > log2_skew1)? log2_skew0: log2_skew1;
        rotate_bits += 1; // XXX
        bitstream_putbits(bs, rotate_bits, 5);
        bitstream_putbits(bs, matrix->rotate_skew0, rotate_bits);
        bitstream_putbits(bs, matrix->rotate_skew1, rotate_bits);
    }
    log2_x = log2f((float) matrix->translate_x);
    log2_y = log2f((float) matrix->translate_y);
    translate_bits = (log2_x > log2_y)? log2_x: log2_y;
    translate_bits += 1; // XXX
    bitstream_putbits(bs, translate_bits, 5);
    bitstream_putbits(bs, matrix->translate_x, translate_bits);
    bitstream_putbits(bs, matrix->translate_y, translate_bits);
    return 0;
}

int
swf_matrix_print(swf_matrix_t *matrix, int indent_depth) {
    print_indent(indent_depth);
    // printf("has_scale=%d\n", matrix->has_scale);
    if (matrix->has_scale) {
        printf("scale=(%5.3f,%5.3f)  ",
               matrix->scale_x / 65536.0, matrix->scale_y / 65536.0);
    }
    // printf("has_rotate=%d\n", matrix->has_scale);
    if (matrix->has_rotate) {
        printf("rotate=(%.2f,%.2f)  ",
               matrix->rotate_skew0 / 65536.0, matrix->rotate_skew1 / 65536.0);
    }
    printf("translate=(%.2f,%.2f)\n",
           (float) matrix->translate_x / SWF_TWIPS, 
	   (float) matrix->translate_y / SWF_TWIPS);
    return 0;
}
