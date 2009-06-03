#include <stdio.h>
#include "bitstream.h"
#include "swf_fill_style_solid.h"

swf_fill_style_solid_parse(bitstream_t *bs,
                           swf_fill_style_solid_t *fill_style_solid,
                           swf_tag_t *tag) {
    if (tag->tag == 46 || tag->tag == 84) { // DefineMorphShape, DefineMorphShape2
        swf_rgba_parse(bs, &(fill_style_solid->rgba));
        swf_rgba_parse(bs, &(fill_style_solid->rgba_morph));
    } else if (tag->tag == 32) { // DefineShape3
        swf_rgba_parse(bs, &(fill_style_solid->rgba));
    } else {
        swf_rgb_parse(bs, &(fill_style_solid->rgb));
    }
    return 0;
}

int
swf_fill_style_solid_build(bitstream_t *bs,
                           swf_fill_style_solid_t *fill_style_solid,
                           swf_tag_t *tag) {
    if (tag->tag == 46 || tag->tag == 84) { // DefineMorphShape, DefineMorphShape2
        swf_rgba_build(bs, &(fill_style_solid->rgba));
        swf_rgba_build(bs, &(fill_style_solid->rgba_morph));
    } else if (tag->tag == 32) { // DefineShape3
        swf_rgba_build(bs, &(fill_style_solid->rgba));
    } else {
        swf_rgb_build(bs, &(fill_style_solid->rgb));
    }
    return 0;
}

int
swf_fill_style_solid_print(swf_fill_style_solid_t *fill_style_solid,
                           int indent_depth, swf_tag_t *tag
) {
    if (tag->tag == 46 || tag->tag == 84) { // DefineMorphShape, DefineMorphShape2
        swf_rgba_print(&(fill_style_solid->rgba), indent_depth + 1);
        swf_rgba_print(&(fill_style_solid->rgba_morph), indent_depth + 1);
    } else if (tag->tag == 32) { // DefineShape3
        swf_rgba_print(&(fill_style_solid->rgba), indent_depth + 1);
    } else {
        swf_rgb_print(&(fill_style_solid->rgb), indent_depth + 1);
    }
    return 0;
}
