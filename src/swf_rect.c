#include <stdio.h>
#include "bitstream.h"
#include "swf_rect.h"

int
swf_rect_parse(bitstream_t *bs, swf_rect_t *rect) {
    int size;
    bitstream_align(bs);
    size = bitstream_getbits(bs, 5);
    if (size == -1) {
        return 1;
    }
    rect->size = size;
    rect->x_min = bitstream_getbits_signed(bs, size);
    rect->x_max = bitstream_getbits_signed(bs, size);
    rect->y_min = bitstream_getbits_signed(bs, size);
    rect->y_max = bitstream_getbits_signed(bs, size);
    return 0;
}

int
swf_rect_build(bitstream_t *bs, swf_rect_t *rect) {
    register unsigned char size;
    unsigned char x_min_bits = bitstream_need_bits_signed(rect->x_min);
    unsigned char x_max_bits = bitstream_need_bits_signed(rect->x_max);
    unsigned char y_min_bits = bitstream_need_bits_signed(rect->y_min);
    unsigned char y_max_bits = bitstream_need_bits_signed(rect->y_max);
    size = (x_min_bits>x_max_bits)?x_min_bits:x_max_bits;
    size = (size>y_min_bits)?size:y_min_bits;
    size = (size>y_max_bits)?size:y_max_bits;
    
    bitstream_align(bs);
    bitstream_putbits(bs, size, 5);
    bitstream_putbits_signed(bs, rect->x_min, size);
    bitstream_putbits_signed(bs, rect->x_max, size);
    bitstream_putbits_signed(bs, rect->y_min, size);
    bitstream_putbits_signed(bs, rect->y_max, size);
    return 0;
}

int
swf_rect_print(swf_rect_t *rect, int indent_depth) {
    print_indent(indent_depth);
    printf("rect=(%d, %d)-(%d, %d) (f_size=%d)\n",
           rect->x_min / SWF_TWIPS,
           rect->y_min / SWF_TWIPS,
           rect->x_max / SWF_TWIPS,
           rect->y_max / SWF_TWIPS,
           rect->size);
    return 0;
}

/* RECT is written with 5bit NBits: up to 31bit signed twips */
#define SWF_RECT_VALUE_MIN (-1073741824.0)
#define SWF_RECT_VALUE_MAX (1073741823.0)

/* trunc(value) + trans (twips), or 1 if a step is not a number that fits */
static int
_swf_rect_value(double value, double trans, signed int *result) {
    if (! ((value >= SWF_RECT_VALUE_MIN) && (value <= SWF_RECT_VALUE_MAX))) {
        return 1; // out of range, or NaN
    }
    value = (double) (signed int) value + trans;
    if (! ((value >= SWF_RECT_VALUE_MIN) && (value <= SWF_RECT_VALUE_MAX))) {
        return 1;
    }
    *result = (signed int) value;
    return 0;
}

/* returns 1, and changes nothing, if a result doesn't fit in a RECT */
int swf_rect_apply_factor(swf_rect_t *rect,
                          double scale_x, double scale_y,
                          signed int trans_x, signed int trans_y) {
    double tx = (double) trans_x * SWF_TWIPS, ty = (double) trans_y * SWF_TWIPS;
    signed int x_min, x_max, y_min, y_max;
    if (_swf_rect_value(rect->x_min, tx, &x_min) ||
        _swf_rect_value(rect->x_min + ((double) rect->x_max - rect->x_min) * scale_x, tx, &x_max) ||
        _swf_rect_value(rect->y_min, ty, &y_min) ||
        _swf_rect_value(rect->y_min + ((double) rect->y_max - rect->y_min) * scale_y, ty, &y_max)) {
        fprintf(stderr, "swf_rect_apply_factor: the result is out of the range of a RECT\n");
        return 1;
    }
    rect->x_min = x_min;
    rect->x_max = x_max;
    rect->y_min = y_min;
    rect->y_max = y_max;
    return 0;
}

/*
 * Like swf_rect_apply_factor(), but every side scales around the point
 * (base_x, base_y) instead of the rect's own (x_min, y_min): the transform the
 * edges of a shape get, for a rect that is not the shape's bounds (EdgeBounds
 * of DefineShape4). With the shape's bounds as the base, the shape's own rect
 * gets the same values as swf_rect_apply_factor().
 */
int swf_rect_apply_factor_around(swf_rect_t *rect,
                                 signed int base_x, signed int base_y,
                                 double scale_x, double scale_y,
                                 signed int trans_x, signed int trans_y) {
    double tx = (double) trans_x * SWF_TWIPS, ty = (double) trans_y * SWF_TWIPS;
    signed int x_min, x_max, y_min, y_max;
    if (_swf_rect_value(base_x + ((double) rect->x_min - base_x) * scale_x, tx, &x_min) ||
        _swf_rect_value(base_x + ((double) rect->x_max - base_x) * scale_x, tx, &x_max) ||
        _swf_rect_value(base_y + ((double) rect->y_min - base_y) * scale_y, ty, &y_min) ||
        _swf_rect_value(base_y + ((double) rect->y_max - base_y) * scale_y, ty, &y_max)) {
        fprintf(stderr, "swf_rect_apply_factor_around: the result is out of the range of a RECT\n");
        return 1;
    }
    rect->x_min = x_min;
    rect->x_max = x_max;
    rect->y_min = y_min;
    rect->y_max = y_max;
    return 0;
}
