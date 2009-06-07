#include <stdio.h>
#include "bitstream.h"
#include "swf_shape_record_setup.h"

int
swf_shape_record_setup_parse(bitstream_t *bs,
                             swf_shape_record_setup_t *shape_record_setup,
                             swf_tag_t *tag, swf_styles_count_t *count) {
    int shape_move_size;
    shape_record_setup->shape_record_type = bitstream_getbit(bs);
    shape_record_setup->shape_has_new_styles = bitstream_getbit(bs);
    shape_record_setup->shape_has_line_style = bitstream_getbit(bs);
    shape_record_setup->shape_has_fill_style1 = bitstream_getbit(bs);
    shape_record_setup->shape_has_fill_style0 = bitstream_getbit(bs);
    shape_record_setup->shape_has_move_to = bitstream_getbit(bs);
    if (shape_record_setup->shape_has_move_to) {
        shape_move_size = bitstream_getbits(bs, 5); // XXX
        shape_record_setup->shape_move_size = shape_move_size;
        shape_record_setup->shape_move_x = bitstream_getbits(bs, shape_move_size);
        shape_record_setup->shape_move_y = bitstream_getbits(bs, shape_move_size);
    }
    if (shape_record_setup->shape_has_fill_style0) {
        shape_record_setup->shape_fill_style0 = bitstream_getbits(bs, count->fill_bits_count);

    }
    if (shape_record_setup->shape_has_fill_style1) {
        shape_record_setup->shape_fill_style1 = bitstream_getbits(bs, count->fill_bits_count);

    }
    if (shape_record_setup->shape_has_line_style) {
        shape_record_setup->shape_line_style = bitstream_getbits(bs, count->line_bits_count);
    }
    if (shape_record_setup->shape_has_new_styles) {
        swf_styles_parse(bs, &(shape_record_setup->styles), tag);
    }
    return 0;
}

int
swf_shape_record_setup_build(bitstream_t *bs,
                             swf_shape_record_setup_t *shape_record_setup,
                             swf_tag_t *tag, swf_styles_count_t *count) {
    int shape_move_size;
    bitstream_putbit(bs, shape_record_setup->shape_record_type);
    bitstream_putbit(bs, shape_record_setup->shape_has_new_styles);
    bitstream_putbit(bs, shape_record_setup->shape_has_line_style);
    bitstream_putbit(bs, shape_record_setup->shape_has_fill_style1);
    bitstream_putbit(bs, shape_record_setup->shape_has_fill_style0);
    bitstream_putbit(bs, shape_record_setup->shape_has_move_to);
    if (shape_record_setup->shape_has_move_to) {
        shape_move_size = shape_record_setup->shape_move_size;
        bitstream_putbits(bs, shape_move_size, 5); // XXX
        bitstream_putbits(bs, shape_record_setup->shape_move_x,
                          shape_move_size);
        bitstream_putbits(bs, shape_record_setup->shape_move_y,
                          shape_move_size);
    }
    if (shape_record_setup->shape_has_fill_style0) {
        bitstream_putbits(bs, shape_record_setup->shape_fill_style0,
                          count->fill_bits_count);
    }
    if (shape_record_setup->shape_has_fill_style1) {
        bitstream_putbits(bs, shape_record_setup->shape_fill_style1,
                          count->fill_bits_count);

    }
    if (shape_record_setup->shape_has_line_style) {
        bitstream_putbits(bs, shape_record_setup->shape_line_style,
                  count->line_bits_count);
    }
    if (shape_record_setup->shape_has_new_styles) {
        swf_styles_build(bs, &(shape_record_setup->styles), tag);
    }
    return 0;
}

int
swf_shape_record_setup_print(swf_shape_record_setup_t *shape_record_setup,
                             int indent_depth, swf_tag_t *tag, 
                             swf_styles_count_t *count) {
    (void) count; // XXX
    if (tag->tag != 2) { // DefineShape
        print_indent(indent_depth);
        printf("shape_record_type=%d\n",
               shape_record_setup->shape_record_type);
    }
    print_indent(indent_depth);
    printf("shape_has_{new_styles,line_styles, fill_style1, fill_style0, move_to}={%d,%d,%d,%d,%d}\n",
           shape_record_setup->shape_has_new_styles,
           shape_record_setup->shape_has_line_style,
           shape_record_setup->shape_has_fill_style1,
           shape_record_setup->shape_has_fill_style0,
           shape_record_setup->shape_has_move_to);
    if (shape_record_setup->shape_has_move_to) {
        print_indent(indent_depth);
        printf("shape_move_x=%d  shape_move_y=%d\n",
               shape_record_setup->shape_move_x,
               shape_record_setup->shape_move_y);
    }
    if (shape_record_setup->shape_has_fill_style0) {
        printf("shape_fill_style0=%d\n",
               shape_record_setup->shape_fill_style0);
    }
    if (shape_record_setup->shape_has_fill_style1) {
        printf("shape_fill_style1=%d\n",
               shape_record_setup->shape_fill_style1);
    }
    if (shape_record_setup->shape_has_line_style) {
        printf("shape_line_style=%d\n",
               shape_record_setup->shape_line_style);
    }
    if (shape_record_setup->shape_has_new_styles) {
        swf_styles_print(&(shape_record_setup->styles), indent_depth + 1,
                         tag);
    }

    return 0;
}
