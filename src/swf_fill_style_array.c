#include <stdio.h>
#include <stdlib.h> // calloc
#include "bitstream.h"
#include "swf_fill_style_array.h"

int
swf_fill_style_array_parse(bitstream_t *bs,
                           swf_fill_style_array_t *shape_with_style,
                           swf_tag_t *tag) {
    int i;
    shape_with_style->count = bitstream_getbyte(bs);
    if (tag->tag != 2 || // ! DefineShape
        shape_with_style->count == 255) {
        shape_with_style->count = bitstream_getbytesLE(bs, 2);
    }
    shape_with_style->fill_style = calloc(shape_with_style->count, sizeof(swf_fill_style_t)); // XXX free !!! XXX
    for (i = 0 ; i < shape_with_style->count ; i++) {
        swf_fill_style_parse(bs, &(shape_with_style->fill_style[i]), tag);
    }
    return 0;
}

int
swf_fill_style_array_build(bitstream_t *bs, swf_fill_style_array_t *shape_with_style, swf_tag_t *tag) {
    int i;
    if (tag->tag != 2 || // ! DefineShape
        255 <= shape_with_style->count) {
        bitstream_putbyte(bs, 255);
        bitstream_putbytesLE(bs, shape_with_style->count, 2);
    } else {
        bitstream_putbyte(bs, shape_with_style->count);
    }
    for (i = 0 ; i < shape_with_style->count ; i++) {
        swf_fill_style_built(bs, &(shape_with_style->fill_style[i]), tag);
    }
}

int
swf_fill_style_array_print(swf_fill_style_array_t *shape_with_style, int indent_depth, swf_tag_t *tag) {
    int i;
    print_indent(indent_depth);
    printf("count=%u\n", shape_with_style->count);
    for (i = 0 ; i < shape_with_style->count ; i++) {
        swf_fill_style_print(&(shape_with_style->fill_style[i]),
                             indent_depth + 1, tag);
    }
    return 0;
}

int
swf_fill_style_array_delete(swf_fill_style_array_t *fill_style_array) {
    free(fill_style_array->fill_style);
}
