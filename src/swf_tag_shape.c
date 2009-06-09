/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_tag_shape.h"
#include "swf_object.h"

swf_tag_detail_handler_t shape_detail_handler;

swf_tag_detail_handler_t *
swf_tag_shape_detail_handler(void) {
    shape_detail_handler.create   = swf_tag_shape_create_detail;
    shape_detail_handler.input    = swf_tag_shape_input_detail;
    shape_detail_handler.identity = swf_tag_shape_identity_detail;
    shape_detail_handler.output   = swf_tag_shape_output_detail;
    shape_detail_handler.print    = swf_tag_shape_print_detail;
    shape_detail_handler.destroy  = swf_tag_shape_destroy_detail;
    return &shape_detail_handler;
}

void *
swf_tag_shape_create_detail(void) {
    swf_tag_shape_detail_t *swf_tag_shape;
    swf_tag_shape = calloc(sizeof(*swf_tag_shape), 1);
    if (swf_tag_shape == NULL) {
        fprintf(stderr, "ERROR: swf_tag_shape_create_detail: can't calloc\n");
        return NULL;
    }
    return swf_tag_shape;
}

int
swf_tag_shape_input_detail(swf_tag_t *tag, struct swf_object_ *swf) {
    swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
    unsigned char *data  = tag->data;
    unsigned long length = tag->length;
    bitstream_t *bs;
    (void) swf;
    if (swf_tag_shape == NULL) {
        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape == NULL\n");
        return 1;
    }
    bs = bitstream_open();
    bitstream_input(bs, data, length);
    swf_tag_shape->shape_id = bitstream_getbytesLE(bs, 2);
    swf_rect_parse(bs, &(swf_tag_shape->rect));
    // DefineMorphShape, DefineMorphShape2
    swf_tag_shape->is_morph = (tag->tag == 46) || (tag->tag == 84);
    // DefineShape4, DefineMorphShape2
    swf_tag_shape->has_strokes = (tag->tag == 83) || (tag->tag == 84);

    if (swf_tag_shape->is_morph) {
        swf_rect_parse(bs, &(swf_tag_shape->rect_morph));
    }
    if (swf_tag_shape->has_strokes) {
        swf_rect_parse(bs, &(swf_tag_shape->stroke_rect));
        if (swf_tag_shape->is_morph) {
            swf_rect_parse(bs, &(swf_tag_shape->stroke_rect_morph));
        }
        swf_tag_shape->define_shape_reserved = bitstream_getbits(bs, 6);
        swf_tag_shape->define_shape_non_scaling_strokes = bitstream_getbits(bs, 1);
        swf_tag_shape->define_shape_scaling_strokes = bitstream_getbits(bs, 1);
    }
    if (swf_tag_shape->is_morph) {
        swf_tag_shape->offset_morph = bitstream_getbytesLE(bs, 4);
        swf_morph_shape_with_style_parse(bs, &swf_tag_shape->morph_shape_with_style, tag);
    } else {
        swf_shape_with_style_parse(bs, &swf_tag_shape->shape_with_style, tag);
    }
        
    bitstream_close(bs);
    return 0;
}

int swf_tag_shape_identity_detail(swf_tag_t *tag, int id) {
    unsigned char *data = tag->data;
    bitstream_t *bs;
    int shape_id;
    if (tag->detail) {
        swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
        if (swf_tag_shape->shape_id == id) {
            return 0;
        }        
        return 1;
    }
    bs = bitstream_open();
    bitstream_input(bs, data, 2);
    shape_id = bitstream_getbytesLE(bs, 2);
    bitstream_close(bs);
    if (id == shape_id) {
        return 0;
    }        
    return 1;
}

unsigned char *
swf_tag_shape_output_detail(swf_tag_t *tag, unsigned long *length,
                           struct swf_object_ *swf) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    bitstream_t *bs;
    unsigned char *data;
    (void) swf;
    *length = 0;
    bs = bitstream_open();
    bitstream_putbytesLE(bs, swf_tag_shape->shape_id, 2);
    swf_rect_build(bs, &(swf_tag_shape->rect));
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

void
swf_tag_shape_print_detail(swf_tag_t *tag,
                           struct swf_object_ *swf, int indent_depth) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    (void) swf;
    print_indent(indent_depth);
    printf("shape_id=%d\n", swf_tag_shape->shape_id);
    swf_rect_print(&(swf_tag_shape->rect), indent_depth);
    print_indent(indent_depth);
    printf("is_morph=%d has_strokes=%d\n",
           swf_tag_shape->is_morph, swf_tag_shape->has_strokes);
    if (swf_tag_shape->is_morph) {
        swf_rect_print(&(swf_tag_shape->rect_morph), indent_depth);
    }
    if (swf_tag_shape->has_strokes) {
        swf_rect_print(&(swf_tag_shape->stroke_rect), indent_depth);
        if (swf_tag_shape->is_morph) {
            swf_rect_print(&(swf_tag_shape->stroke_rect_morph), indent_depth);
        }
        print_indent(indent_depth);
        printf("define_shape_non_scaling_strokes=%d define_shape_scaling_strokes=%d\n",
               swf_tag_shape->define_shape_non_scaling_strokes,
               swf_tag_shape->define_shape_scaling_strokes);
    }
    if (swf_tag_shape->is_morph) {
        print_indent(indent_depth);
        printf("offset_morph=%lu\n", swf_tag_shape->offset_morph);
        swf_morph_shape_with_style_print(&swf_tag_shape->morph_shape_with_style,
                                         indent_depth, tag);
    } else {
        swf_shape_with_style_print(&swf_tag_shape->shape_with_style,
                                   indent_depth, tag);
    }
    return ;
}

void
swf_tag_shape_destroy_detail(swf_tag_t *tag) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    if (swf_tag_shape) {
        swf_morph_shape_with_style_delete(&(swf_tag_shape->morph_shape_with_style));
        free(swf_tag_shape);
    }
    return ;
}
