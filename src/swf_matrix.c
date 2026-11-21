#include <stdio.h>
#include <math.h>
#include "bitstream.h"
#include "swf_matrix.h"

int
swf_matrix_parse(bitstream_t *bs, swf_matrix_t *matrix) {
    int translate_bits;
    int ret;
    bitstream_align(bs); // The MATRIX record must be byte aligned.
    ret = bitstream_getbit(bs);
    if (ret == -1) {
        return 1;
    }
    matrix->has_scale = ret;
    if (matrix->has_scale) {
        int scale_bits = bitstream_getbits(bs, 5);
        matrix->scale_bits = scale_bits;
        matrix->scale_x = bitstream_getbits_signed(bs, scale_bits);
        matrix->scale_y = bitstream_getbits_signed(bs, scale_bits);
    } else {
        matrix->scale_x = SWF_TWIPS; // XXX
        matrix->scale_y = SWF_TWIPS; // XXX
    }
    ret = bitstream_getbit(bs);
    if (ret == -1) {
        return 1;
    }
    matrix->has_rotate = ret;
    if (matrix->has_rotate) {
        int rotate_bits = bitstream_getbits(bs, 5);
        matrix->rotate_bits = rotate_bits;
        matrix->rotate_skew0 = bitstream_getbits_signed(bs, rotate_bits);
        matrix->rotate_skew1 = bitstream_getbits_signed(bs, rotate_bits);
    } else {
        matrix->rotate_skew0 = 0;
        matrix->rotate_skew1 = 0;
    }
    translate_bits = bitstream_getbits(bs, 5);
    matrix->translate_bits = translate_bits;
    matrix->translate_x = bitstream_getbits_signed(bs, translate_bits);
    matrix->translate_y = bitstream_getbits_signed(bs, translate_bits);
    return 0;
}

int
swf_matrix_build(bitstream_t *bs, swf_matrix_t *matrix) {
    int translate_bits;
    float log2_x, log2_y;
    bitstream_align(bs); // The MATRIX record must be byte aligned.
    bitstream_putbit(bs, matrix->has_scale);
    if (matrix->has_scale) {
        int scale_bits;
        log2_x = bitstream_need_bits_signed(matrix->scale_x);
        log2_y = bitstream_need_bits_signed(matrix->scale_y);
        scale_bits = (log2_x > log2_y)? log2_x: log2_y;
        bitstream_putbits(bs, scale_bits, 5);
        bitstream_putbits_signed(bs, matrix->scale_x, scale_bits);
        bitstream_putbits_signed(bs, matrix->scale_y, scale_bits);
    }
    if ((matrix->rotate_skew0 == 0) && (matrix->rotate_skew1 == 0)) {
        matrix->has_rotate = 0;
    }
    bitstream_putbit(bs, matrix->has_rotate);
    if (matrix->has_rotate) {
        int rotate_bits;
        float log2_skew0, log2_skew1;
        log2_skew0 = bitstream_need_bits_signed(matrix->rotate_skew0);
        log2_skew1 = bitstream_need_bits_signed(matrix->rotate_skew1);
        rotate_bits = (log2_skew0 > log2_skew1)? log2_skew0: log2_skew1;
        bitstream_putbits(bs, rotate_bits, 5);
        bitstream_putbits_signed(bs, matrix->rotate_skew0, rotate_bits);
        bitstream_putbits_signed(bs, matrix->rotate_skew1, rotate_bits);
    }
    if ((matrix->translate_x == 0) && (matrix->translate_y == 0)) {
        translate_bits = 0;
        bitstream_putbits(bs, translate_bits, 5);
    } else {
        log2_x = bitstream_need_bits_signed(matrix->translate_x);
        log2_y = bitstream_need_bits_signed(matrix->translate_y);
        translate_bits = (log2_x > log2_y)? log2_x: log2_y;
        bitstream_putbits(bs, translate_bits, 5);
        bitstream_putbits_signed(bs, matrix->translate_x, translate_bits);
        bitstream_putbits_signed(bs, matrix->translate_y, translate_bits);
    }
    return 0;
}

int
swf_matrix_print(swf_matrix_t *matrix, int indent_depth) {
    print_indent(indent_depth);
    if (matrix->has_scale) {
        printf("scale=(%.3f,%.3f):bits=%u  ",
               matrix->scale_x / 65536.0 / SWF_TWIPS,
	       matrix->scale_y / 65536.0 / SWF_TWIPS,
               matrix->scale_bits);
    } else {
        printf("(has_scale=no)  ");
    }
    if (matrix->has_rotate) {
        printf("rotate=(%.3f,%.3f)bits=%u\n",
               matrix->rotate_skew0 / 65536.0 / SWF_TWIPS,
	       matrix->rotate_skew1 / 65536.0 / SWF_TWIPS,
               matrix->rotate_bits);
    } else {
        printf("(has_rotate=no)\n");
    }
    print_indent(indent_depth);
    printf("translate=(%.2f,%.2f):bits=%u\n",
           (float) matrix->translate_x / SWF_TWIPS,
	   (float) matrix->translate_y / SWF_TWIPS,
           matrix->translate_bits);
    return 0;
}

/* the values of a MATRIX are written with 5bit NBits: up to 31bit signed
 * (n.16 fixed point, twips) */
static int
_swf_matrix_value_fits(double v) {
    return (v >= -1073741824.0) && (v <= 1073741823.0); // false for NaN
}

/* returns 1, and changes nothing, if a result doesn't fit in a MATRIX */
int
swf_matrix_apply_factor(swf_matrix_t *matrix,
                        double scale_x, double scale_y,
                        double rotate_rad,
                        signed int trans_x, signed int trans_y) {
    /* x' = ScaleX * x + RotateSkew1 * y, y' = RotateSkew0 * x + ScaleY * y:
     * scaling the result (shape coordinates) scales each row */
    double sx, sy, s0, s1, tx, ty;
    sx = (matrix->has_scale ? matrix->scale_x : 65536) * scale_x; // 1.0 when the scale is omitted
    s1 = matrix->rotate_skew1 * scale_x;
    s0 = matrix->rotate_skew0 * scale_y;
    sy = (matrix->has_scale ? matrix->scale_y : 65536) * scale_y;
    if (! (_swf_matrix_value_fits(sx) && _swf_matrix_value_fits(s1) &&
           _swf_matrix_value_fits(s0) && _swf_matrix_value_fits(sy))) {
        goto out_of_range;
    }
    sx = (double) (signed long) sx; s1 = (double) (signed long) s1;
    s0 = (double) (signed long) s0; sy = (double) (signed long) sy;
    if ((rotate_rad != 0)) {
        double rot_sin, rot_cos, rx, ry, r0, r1;
        if (matrix->has_rotate == 0) {
            s0 = 0;
            s1 = 0;
        }
        rot_cos = cos(rotate_rad);
        rot_sin = sin(rotate_rad);
        rx = rot_cos * sx - rot_sin * s0;
        r0 = rot_sin * sx + rot_cos * s0;
        r1 = rot_cos * s1 - rot_sin * sy;
        ry = rot_sin * s1 + rot_cos * sy;
        if (! (_swf_matrix_value_fits(rx) && _swf_matrix_value_fits(r0) &&
               _swf_matrix_value_fits(r1) && _swf_matrix_value_fits(ry))) {
            goto out_of_range;
        }
        sx = rx; sy = ry; s0 = r0; s1 = r1;
    }
    tx = matrix->translate_x + 20.0 * trans_x;
    ty = matrix->translate_y + 20.0 * trans_y;
    if (! (_swf_matrix_value_fits(tx) && _swf_matrix_value_fits(ty))) {
        goto out_of_range;
    }
    matrix->has_scale = 1;
    if ((rotate_rad != 0)) {
        matrix->has_rotate = 1;
    }
    matrix->scale_x = (signed long) sx;
    matrix->scale_y = (signed long) sy;
    matrix->rotate_skew0 = (signed long) s0;
    matrix->rotate_skew1 = (signed long) s1;
    matrix->translate_x = (signed) tx;
    matrix->translate_y = (signed) ty;
    return 0;
out_of_range:
    fprintf(stderr, "swf_matrix_apply_factor: the result is out of the range of a MATRIX\n");
    return 1;
}

/* scale on the source side (e.g. bitmap pixels): scales each column.
 * returns 1, and changes nothing, if a result doesn't fit in a MATRIX */
int
swf_matrix_scale_source(swf_matrix_t *matrix, double scale_x, double scale_y) {
    double sx, sy, s0, s1;
    sx = (matrix->has_scale ? matrix->scale_x : 65536) * scale_x;
    s0 = matrix->rotate_skew0 * scale_x;
    s1 = matrix->rotate_skew1 * scale_y;
    sy = (matrix->has_scale ? matrix->scale_y : 65536) * scale_y;
    if (! (_swf_matrix_value_fits(sx) && _swf_matrix_value_fits(s1) &&
           _swf_matrix_value_fits(s0) && _swf_matrix_value_fits(sy))) {
        fprintf(stderr, "swf_matrix_scale_source: the result is out of the range of a MATRIX\n");
        return 1;
    }
    matrix->has_scale = 1;
    matrix->scale_x = (signed long) sx;
    matrix->rotate_skew0 = (signed long) s0;
    matrix->rotate_skew1 = (signed long) s1;
    matrix->scale_y = (signed long) sy;
    return 0;
}
