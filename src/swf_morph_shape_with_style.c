#include <stdio.h>
#include "bitstream.h"
#include "swf_morph_shape_with_style.h"

swf_morph_shape_with_style_parse(bitstream_t *bs,
                                 swf_morph_shape_with_style_t *morph_shape_with_style,
                                 swf_tag_t *tag) {
    swf_styles_parse(bs, &(morph_shape_with_style->styles), tag);
    swf_shape_record_parse(bs, &(morph_shape_with_style->shape_records));
    swf_styles_count_parse(bs, &(morph_shape_with_style->styles_count));
    swf_shape_record_parse(bs, &(morph_shape_with_style->shape_records_morph));
    return 0;
}

int
swf_morph_shape_with_style_build(bitstream_t *bs,
                                 swf_morph_shape_with_style_t *morph_shape_with_style,
                                 swf_tag_t *tag) {
    swf_styles_build(bs, &(morph_shape_with_style->styles), tag);
    swf_shape_record_build(bs, &(morph_shape_with_style->shape_records));
    swf_styles_count_build(bs, &(morph_shape_with_style->styles_count));
    swf_shape_record_build(bs, &(morph_shape_with_style->shape_records_morph));
    return 0;
}

int
swf_morph_shape_with_style_print(swf_morph_shape_with_style_t *morph_shape_with_style,
                                 int indent_depth,
                                 swf_tag_t *tag) {
    swf_styles_print(&(morph_shape_with_style->styles), indent_depth, tag);
    swf_shape_record_print(&(morph_shape_with_style->shape_records), indent_depth);
    swf_styles_count_print(&(morph_shape_with_style->styles_count), indent_depth);
    swf_shape_record_print(&(morph_shape_with_style->shape_records_morph), indent_depth);
    return 0;
}

int
swf_morph_shape_with_style_delete(swf_morph_shape_with_style_t *morph_shape_with_style) {
    swf_styles_delete(&(morph_shape_with_style->styles));
}
