#include <stdio.h>
#include "bitstream.h"
#include "swf_fill_style_array.h"

int
swf_fill_style_array_parse(bitstream_t *bs,
                           swf_fill_style_array_t *shape_with_style,
                           swf_tag_t *tag) {
    int i;
    shape_with_style->count = bitstream_getbyte(bs);
    if (tag->tag != 2 || // DefineShape
        shape_with_style->count == 255) {
        shape_with_style->count = bitstream_getbytesLE(bs, 2);
    }
    for (i = 0 ; i < shape_with_style->count ; i++) {
        swf_fill_style_parse(bs, &(shape_with_style->fill_style), tag);
    }
    return 0;
}

int
swf_fill_style_array_build(bitstream_t *bs, swf_fill_style_array_t *shape_with_style) {
//    swf_fill_style_array_build(bs, &(shape_with_style->fill_styles));
//    swf_line_style_array_build(bs, &(shape_with_style->line_styles));
//    swf_fill_style_array_count_build(bs, &(shape_with_style->line_styles));
    return 0;
}

int
swf_fill_style_array_print(swf_fill_style_array_t *shape_with_style, int indent_depth) {
//    swf_fill_style_array_print(&(shape_with_style->fill_styles), indent_depth);
//    swf_line_style_array_print(&(shape_with_style->line_styles), indent_depth);
//    swf_fill_style_array_count_print(&(shape_with_style->line_styles), indent_depth);
    return 0;
}
