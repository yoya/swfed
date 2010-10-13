#include <stdio.h>
#include "bitstream.h"
#include "swf_styles.h"
#include "swf_tag_shape.h"

int
swf_styles_parse(bitstream_t *bs, swf_styles_t *shape_with_style,
                 swf_tag_t *tag) {
    int result;
    result = swf_fill_style_array_parse(bs, &(shape_with_style->fill_styles),
                                        tag);
    if (result) {
        fprintf(stderr, "swf_styles_parse: swf_fill_style_array_parse failed\n");
        return result;
    }
    result = swf_line_style_array_parse(bs, &(shape_with_style->line_styles),
                                        tag);
    if (result) {
        fprintf(stderr, "swf_styles_parse: swf_line_style_array_parse failed\n");
        return result;
    }
    result = swf_styles_count_parse(bs, &(shape_with_style->styles_count));
    if (result) {
        fprintf(stderr, "swf_styles_parse: swf_styles_count_parse failed\n");
        return result;
    }

    return result;
}

int
swf_styles_build(bitstream_t *bs, swf_styles_t *shape_with_style,
                 swf_tag_t *tag) {
    int ret;
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    swf_styles_count_t *count = &(swf_tag_shape->_current_styles_count);
    ret = swf_fill_style_array_build(bs, &(shape_with_style->fill_styles), tag);
    if (ret) {
        fprintf(stderr, "swf_styles_build: swf_fill_style_array_build failed");
        return ret;
    }
    ret = swf_line_style_array_build(bs, &(shape_with_style->line_styles), tag);
    if (ret) {
        fprintf(stderr, "swf_styles_build: swf_line_style_array_build failed");
        return ret;
    }
    count->fill_bits_count += shape_with_style->fill_styles.count; // XXX
    count->line_bits_count += shape_with_style->line_styles.count; // XXX
    ret = swf_styles_count_build(bs, &(shape_with_style->styles_count));
    if (ret) {
        fprintf(stderr, "swf_styles_build: swf_styles_count_build failed");
        return ret;
    }
    return 0;
}

int
swf_styles_print(swf_styles_t *shape_with_style, int indent_depth,
                 swf_tag_t *tag) {
    swf_fill_style_array_print(&(shape_with_style->fill_styles), indent_depth,
        tag);
    swf_line_style_array_print(&(shape_with_style->line_styles), indent_depth,
        tag);
    swf_styles_count_print(&(shape_with_style->styles_count), indent_depth);
    return 0;
}

int
swf_styles_delete(swf_styles_t *styles) {
    swf_fill_style_array_delete(&(styles->fill_styles));
    swf_line_style_array_delete(&(styles->line_styles));
    return 0;
}
