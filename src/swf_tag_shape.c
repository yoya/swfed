/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_shape_record.h"
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
    int ret;
    (void) swf;
    if (swf_tag_shape == NULL) {
        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape == NULL\n");
        return 1;
    }
    // parse context
    swf_tag_shape->_current_x = 0;
    swf_tag_shape->_current_y = 0;
    swf_tag_shape->_parse_condition = 0;

    bs = bitstream_open();
    bitstream_input(bs, data, length);
    
    swf_tag_shape->shape_id = bitstream_getbytesLE(bs, 2);
    ret = swf_rect_parse(bs, &(swf_tag_shape->rect));
    if (ret) {
        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->rect parse failed. shape_id=%d\n", swf_tag_shape->shape_id);
        bitstream_close(bs);
        return ret;
    }

    // DefineMorphShape, DefineMorphShape2
    swf_tag_shape->is_morph = (tag->tag == 46) || (tag->tag == 84);
    // DefineShape4, DefineMorphShape2
    swf_tag_shape->has_strokes = (tag->tag == 83) || (tag->tag == 84);

    if (swf_tag_shape->is_morph) {
        ret = swf_rect_parse(bs, &(swf_tag_shape->rect_morph));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->rect_morph parse failed. shape_id=%d\n", swf_tag_shape->shape_id);
            bitstream_close(bs);
            return ret;
        }
    }
    if (swf_tag_shape->has_strokes) {
        ret = swf_rect_parse(bs, &(swf_tag_shape->stroke_rect));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->stroke_rect parse failed. shape_id=%d\n", swf_tag_shape->shape_id);
            bitstream_close(bs);
            return ret;
        }
        if (swf_tag_shape->is_morph) {
            ret = swf_rect_parse(bs, &(swf_tag_shape->stroke_rect_morph));
	    if (ret) {
	        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->stroke_rect_morph parse failed. shape_id=%d\n", swf_tag_shape->shape_id);
	        bitstream_close(bs);
	        return ret;
	    }
        }
        swf_tag_shape->define_shape_reserved = bitstream_getbits(bs, 6);
        swf_tag_shape->define_shape_non_scaling_strokes = bitstream_getbits(bs, 1);
        swf_tag_shape->define_shape_scaling_strokes = bitstream_getbits(bs, 1);
    }
    if (swf_tag_shape->is_morph) {
        swf_tag_shape->offset_morph = bitstream_getbytesLE(bs, 4);
        swf_morph_shape_with_style_parse(bs, &swf_tag_shape->morph_shape_with_style, tag);
    } else {
        ret = swf_shape_with_style_parse(bs, &swf_tag_shape->shape_with_style, tag);
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_shape_with_style_parse swf_tag_shape->shape_with_style failed. shape_id=%d\n", swf_tag_shape->shape_id);
	        bitstream_close(bs);
                return ret;
        }
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

int swf_tag_shape_bitmap_identity(swf_tag_t *tag, int bitmap_id) {
    swf_tag_shape_detail_t *swf_tag_shape;
    int i, ret;
    if (tag == NULL) {
        fprintf(stderr, "swf_tag_shape_bitmap_identity: tag == NULL\n");
        return 1;
    }
    if (tag->detail == NULL) {
        tag->detail = swf_tag_shape_create_detail();
        swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
        swf_tag_shape->_parse_condition = SWF_TAG_SHAPE_PARSE_CONDITION_BITMAP;
        ret = swf_tag_shape_input_detail(tag, NULL);
        if (ret) {
            swf_tag_shape_destroy_detail(tag);
            return 1; // no shape bitmap
        }
    } else {
        swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    }
    //
    for (i = 0 ; i < swf_tag_shape->shape_with_style.styles.fill_styles.count ; i++) {
        swf_fill_style_t *fill_style;
        fill_style = &(swf_tag_shape->shape_with_style.styles.fill_styles.fill_style[i]);
        switch (fill_style->type) {
          case 0x40: // tilled  bitmap fill with smoothed edges
          case 0x41: // clipped bitmap fill with smoothed edges
          case 0x42: // tilled  bitmap fill with hard edges
          case 0x43: // clipped bitmap fill with hard edges
            if (fill_style->bitmap.bitmap_ref == bitmap_id) {
                return 0; // found
            }
            break;
          default:
            break;
        }
    }
    return 1; // not found
}


unsigned char *
swf_tag_shape_output_detail(swf_tag_t *tag, unsigned long *length,
                           struct swf_object_ *swf) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    bitstream_t *bs;
    unsigned char *data;
    int ret;
    (void) swf;
    *length = 0;
    // build context
    swf_tag_shape->_current_fill_style_num = 0;
    swf_tag_shape->_current_line_style_num = 0;
    swf_tag_shape->_current_x = 0;
    swf_tag_shape->_current_y = 0;

    //
    bs = bitstream_open();
    bitstream_putbytesLE(bs, swf_tag_shape->shape_id, 2);
    swf_rect_build(bs, &(swf_tag_shape->rect));

    // DefineMorphShape, DefineMorphShape2
    swf_tag_shape->is_morph = (tag->tag == 46) || (tag->tag == 84);
    // DefineShape4, DefineMorphShape2
    swf_tag_shape->has_strokes = (tag->tag == 83) || (tag->tag == 84);

    if (swf_tag_shape->is_morph) {
        ret = swf_rect_build(bs, &(swf_tag_shape->rect_morph));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_shape_output_detail: swf_tag_shape->rect_morph build failed\n");
            bitstream_close(bs);
            return NULL;
        }
    }
    if (swf_tag_shape->has_strokes) {
        ret = swf_rect_build(bs, &(swf_tag_shape->stroke_rect));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->stroke_rect build failed\n");
            bitstream_close(bs);
            return NULL;
        }
        if (swf_tag_shape->is_morph) {
            ret = swf_rect_build(bs, &(swf_tag_shape->stroke_rect_morph));
	    if (ret) {
	        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape->stroke_rect_morph build failed\n");
	        bitstream_close(bs);
	        return NULL;
	    }
        }
        bitstream_putbits(bs, 6, swf_tag_shape->define_shape_reserved );
        bitstream_putbits(bs, 1, swf_tag_shape->define_shape_non_scaling_strokes );
        bitstream_putbits(bs, 1, swf_tag_shape->define_shape_scaling_strokes);
    }
    if (swf_tag_shape->is_morph) {
        bitstream_putbytesLE(bs, 4, swf_tag_shape->offset_morph);
        swf_morph_shape_with_style_build(bs, &swf_tag_shape->morph_shape_with_style, tag);
    } else {
        ret = swf_shape_with_style_build(bs, &swf_tag_shape->shape_with_style, tag);
        if (ret) {
            fprintf(stderr, "swf_tag_shape_output_detail: swf_shape_with_style_build failed\n");
            bitstream_close(bs);
            return NULL;
        }
    }
    
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
//    printf("is_morph=%d has_strokes=%d\n",
//           swf_tag_shape->is_morph, swf_tag_shape->has_strokes);
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
        swf_shape_with_style_delete(&(swf_tag_shape->shape_with_style));
        free(swf_tag_shape);
    }
    return ;
}

int
swf_tag_shape_apply_matrix_factor(void *detail, int shape_id,
                                  double scale_x, double scale_y,
                                  double rotate_rad,
                                  signed int trans_x,
                                  signed int trans_y) {
    int i;
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) detail;
    if (detail == NULL) {
        fprintf(stderr, "swf_tag_shape_apply_matrix_factor: detail == NULL\n");
        return 1;
    }
    swf_fill_style_t *fill_style;
    if (shape_id != swf_tag_shape->shape_id) {
        return 1;
    }
    for (i = 0 ; i < swf_tag_shape->shape_with_style.styles.fill_styles.count ; i++) {
        fill_style = &(swf_tag_shape->shape_with_style.styles.fill_styles.fill_style[i]);
        switch (fill_style->type) {
          case 0x10: // linear gradient fill
          case 0x12: // radial gradient fill
          case 0x13: // focal  gradient fill
              swf_matrix_apply_factor(&(fill_style->gradient.gradient_matrix), scale_x, scale_y,
                                      rotate_rad, trans_x, trans_y);
              break;
          case 0x40: // tilled  bitmap fill with smoothed edges
          case 0x41: // clipped bitmap fill with smoothed edges
          case 0x42: // tilled  bitmap fill with hard edges
          case 0x43: // clipped bitmap fill with hard edges
              swf_matrix_apply_factor(&(fill_style->bitmap.bitmap_matrix),
                                      scale_x, scale_y, rotate_rad,
                                      trans_x, trans_y);
              break;
          default:
            fprintf(stderr, "swf_tag_shape_apply_matrix_factor: unknown fill_style->type=%d\n",
                    fill_style->type);
            return 1;
        }
    }
    return 0;
}

int
swf_tag_shape_apply_rect_factor(void *detail, int shape_id,
                                double scale_x, double scale_y,
                                signed int trans_x,
                                signed int trans_y) {
//    int i;
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) detail;
    if (detail == NULL) {
        fprintf(stderr, "swf_tag_shape_apply_matrix_factor: detail == NULL\n");
        return 1;
    }
    if (shape_id != swf_tag_shape->shape_id) {
        return 1;
    }
    swf_rect_apply_factor(&(swf_tag_shape->rect),
                          scale_x, scale_y, trans_x, trans_y);
    
    swf_shape_record_edge_apply_factor(&(swf_tag_shape->shape_with_style.shape_records),
                                       scale_x, scale_y, trans_x, trans_y);
    return 0;
}
