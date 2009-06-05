#include <stdio.h>
#include "bitstream.h"
#include "swf_line_style.h"

swf_line_style_parse(bitstream_t *bs, swf_line_style_t *line_style,
                     swf_tag_t *tag) {
    if (tag->tag == 46) { // DefineMorphShape
        line_style->width = bitstream_getbytesLE(bs, 2);
        line_style->width_morph = bitstream_getbytesLE(bs, 2);
        swf_rgba_parse(bs, &(line_style->rgba));
        swf_rgba_parse(bs, &(line_style->rgba_morph));
    } else if (tag->tag == 83 || tag->tag == 84) {
        // DefineShape4 || DefineMorphShape2
        if (tag->tag == 84) { // DefineMorphShape2
            line_style->width_morph = bitstream_getbytesLE(bs, 2);
        }
        line_style->start_cap_style = bitstream_getbits(bs, 2);
        line_style->join_style = bitstream_getbits(bs, 2);
        line_style->has_fill = bitstream_getbits(bs, 1);
        line_style->no_hscale = bitstream_getbits(bs, 1);
        line_style->no_vscale = bitstream_getbits(bs, 1);
        line_style->pixel_hinting = bitstream_getbits(bs, 1);
        line_style->reserved = bitstream_getbits(bs, 5);
        line_style->no_close = bitstream_getbits(bs, 1);
        line_style->end_cap_style = bitstream_getbits(bs, 2);
        if (line_style->join_style == 2) {
            line_style->miter_limit_factor = bitstream_getbytesLE(bs, 2);
        }
        if (line_style->has_fill) {
            swf_fill_style_parse(bs, &(line_style->fill_style), tag);
        } else {
            swf_rgba_parse(bs, &(line_style->rgba));
            if (tag->tag == 84) { // DefineMorphShape2
                swf_rgba_parse(bs, &(line_style->rgba_morph));
            }
        }
    } else if (tag->tag == 32) { // DefineShape3
        line_style->width = bitstream_getbytesLE(bs, 2);
        swf_rgba_parse(bs, &(line_style->rgba));
    } else {
        line_style->width = bitstream_getbytesLE(bs, 2);
        swf_rgb_parse(bs, &(line_style->rgb));
    }
    return 0;
}

int
swf_line_style_build(bitstream_t *bs, swf_line_style_t *line_style,
                     swf_tag_t *tag) {
    if (tag->tag == 46) { // DefineMorphShape
        bitstream_putbytesLE(bs, line_style->width, 2);
        bitstream_putbytesLE(bs, line_style->width_morph, 2);
        swf_rgba_build(bs, &(line_style->rgba));
        swf_rgba_build(bs, &(line_style->rgba_morph));
    } else if (tag->tag == 83 || tag->tag == 84) {
        // DefineShape4 || DefineMorphShape2
        if (tag->tag == 84) { // DefineMorphShape2
            bitstream_putbytesLE(bs, line_style->width_morph, 2);
        }
        bitstream_putbits(bs, line_style->start_cap_style, 2);
        bitstream_putbits(bs, line_style->join_style, 2);
        bitstream_putbits(bs, line_style->has_fill, 1);
        bitstream_putbits(bs, line_style->no_hscale, 1);
        bitstream_putbits(bs, line_style->no_vscale, 1);
        bitstream_putbits(bs, line_style->pixel_hinting, 1);
        bitstream_putbits(bs, line_style->reserved , 5);
        bitstream_putbits(bs, line_style->no_close, 1);
        bitstream_putbits(bs, line_style->end_cap_style, 2);
        if (line_style->join_style == 2) {
            bitstream_putbytesLE(bs, line_style->miter_limit_factor, 2);
        }
        if (line_style->has_fill) {
            swf_fill_style_build(bs, &(line_style->fill_style), tag);
        } else {
            swf_rgba_build(bs, &(line_style->rgba));
            if (tag->tag == 84) { // DefineMorphShape2
                swf_rgba_build(bs, &(line_style->rgba_morph));
            }
        }
    } else if (tag->tag == 32) { // DefineShape3
        bitstream_putbytesLE(bs, line_style->width, 2);
        swf_rgba_build(bs, &(line_style->rgba));
    } else {
        bitstream_putbytesLE(bs, line_style->width, 2);
        swf_rgb_build(bs, &(line_style->rgb));
    }
    return 0;
}

int
swf_line_style_print(swf_line_style_t *line_style, int indent_depth,
                     swf_tag_t *tag) {
    ;
    return 0;
}
