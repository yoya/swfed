#include <stdio.h>
#include "bitstream.h"
#include "swf_styles.h"

int
swf_styles_parse(bitstream_t *bs, swf_styles_t *shape_with_style,
                 swf_tag_t *tag) {
    int result;
    result = swf_fill_style_array_parse(bs, &(shape_with_style->fill_styles),
                                        tag);
    if (result) {
        return result;
    }
    result = swf_line_style_array_parse(bs, &(shape_with_style->line_styles),
                                        tag);
    if (result) {
        return result;
    }
    result = swf_styles_count_parse(bs, &(shape_with_style->styles_count));
    return result;
}

int
swf_styles_build(bitstream_t *bs, swf_styles_t *shape_with_style,
                 swf_tag_t *tag) {
    swf_fill_style_array_build(bs, &(shape_with_style->fill_styles), tag);
    swf_line_style_array_build(bs, &(shape_with_style->line_styles), tag);
    swf_styles_count_build(bs, &(shape_with_style->styles_count));
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
