#include <stdio.h>
#include "bitstream.h"
#include "swf_shape_record.h"

int
swf_shape_record_parse(bitstream_t *bs, swf_shape_record_t *shape_record,
                       swf_tag_t *tag) {
    int first_bit, next_5bits;
    swf_shape_record_t *current_record = shape_record;
    int limit;
    for (limit = 1; current_record ; limit ++) {
        int ret;
        current_record->next = NULL; // stopper
        ret = bitstream_getbits(bs, 6);
        if (ret == -1) {
            fprintf(stderr, "swf_shape_record_parse: bitstream_getbits failed at L%d\n", __LINE__);
            return ret;
        }
        current_record->first_6bits = ret;
        first_bit = (current_record->first_6bits >> 5) & 1;
        next_5bits = current_record->first_6bits & 0x1f;
        bitstream_incrpos(bs, 0, -6); // 6 bit back
        if ((first_bit == 0) && (next_5bits == 0)) {
            ret = swf_shape_record_end_parse(bs, &(current_record->shape.shape_end));
            if (ret) {
                fprintf(stderr, "swf_shape_record_parse: swf_shape_record_end_parse at L%d\n", __LINE__);
                return ret;
            }
            break; // end
        } if (first_bit == 0) {
            ret = swf_shape_record_setup_parse(bs, &(current_record->shape.shape_setup),
                                         tag);
            if (ret) {
                fprintf(stderr, "swf_shape_record_parse: swf_shape_record_setup_parse at L%d\n", __LINE__);
                return ret;
            }
} else {
            ret = swf_shape_record_edge_parse(bs, &(current_record->shape.shape_edge), tag);
            if (ret) {
                fprintf(stderr, "swf_shape_record_parse: swf_shape_record_edge_parse at L%d\n", __LINE__);
                return ret;
            }

        }
        if (100000 <= limit) { // XXX 100000???
            current_record->next = NULL;
            fprintf(stderr, "swf_shape_record_parse: limit(%d) over\n", limit);
            return 1;
        }
        current_record->next = calloc(1, sizeof(swf_shape_record_t));
        current_record = current_record->next;
    }
    return 0;
}

int
swf_shape_record_build(bitstream_t *bs, swf_shape_record_t *shape_record,
                       swf_tag_t *tag) {
    int first_bit, next_5bits;
    swf_shape_record_t *current_record = shape_record;
    while (current_record) {
        first_bit = (current_record->first_6bits >> 5) & 1;
        next_5bits = current_record->first_6bits & 0x1f;
        if ((first_bit == 0) && (next_5bits == 0)) {
            swf_shape_record_end_build(bs, &(current_record->shape.shape_end));
            break;
        } if (first_bit == 0) {
            swf_shape_record_setup_build(bs, &(current_record->shape.shape_setup),
                                         tag);
        } else {
            swf_shape_record_edge_build(bs, &(current_record->shape.shape_edge), tag);
        }
        current_record = current_record->next;
    }
    return 0;
}

int
swf_shape_record_print(swf_shape_record_t *shape_record, int indent_depth,
                       swf_tag_t *tag) {
    int first_bit, next_5bits;
    swf_shape_record_t *current_record = shape_record;
    int i;
    for (i = 0 ; current_record ; i++) {
        first_bit = (current_record->first_6bits >> 5) & 1;
        next_5bits = current_record->first_6bits & 0x1f;
        print_indent(indent_depth);
        printf("shape_record [%d]  ", i);
        if ((first_bit == 0) && (next_5bits == 0)) {
            printf("end\n");
            swf_shape_record_end_print(indent_depth + 1);
            break;
        } if (first_bit == 0) {
            printf("setup\n");
            swf_shape_record_setup_print(&(current_record->shape.shape_setup),
                                         indent_depth + 1, tag);
        } else {
            printf("edge\n");
            swf_shape_record_edge_print(&(current_record->shape.shape_edge),
                                        indent_depth + 1);
        }
        current_record = current_record->next;
    }
    return 0;
}

int
swf_shape_record_delete(swf_shape_record_t *shape_record) {
    swf_shape_record_t *current, *next;
    int first_bit, next_5bits;
    
    first_bit = (shape_record->first_6bits >> 5) & 1;
    next_5bits = shape_record->first_6bits & 0x1f;

    if ((first_bit == 0) && (next_5bits != 0)) {
        swf_shape_record_setup_delete(&(shape_record->shape.shape_setup));
    }

    current = shape_record->next;
    while (current) {
        first_bit = (current->first_6bits >> 5) & 1;
        next_5bits = current->first_6bits & 0x1f;
        if ((first_bit == 0) && (next_5bits != 0)) {
            swf_shape_record_setup_delete(&(current->shape.shape_setup));
        }
        next = current->next;
        free(current);
        current = next;
    }
    return 0;
}

int
swf_shape_record_edge_apply_factor(swf_shape_record_t *shape_record,
                                   signed base_x, signed base_y,
                                   double scale_x, double scale_y,
                                   signed trans_x, signed trans_y) {
    int pass;
#define SCALE_X(v) ((signed) (((v) - base_x) * scale_x) + base_x + trans_x * SWF_TWIPS)
#define SCALE_Y(v) ((signed) (((v) - base_y) * scale_y) + base_y + trans_y * SWF_TWIPS)
    /* pass 0 checks that every edge delta still fits in 17 bits (NumBits is
     * 4 bits + 2), pass 1 applies; so a failure changes nothing */
    for (pass = 0 ; pass < 2 ; pass++) {
        swf_shape_record_t *current_record;
        signed pen_x = 0, pen_y = 0;
        for (current_record = shape_record ; current_record ; current_record = current_record->next) {
            int first_bit = (current_record->first_6bits >> 5) & 1;
            int next_5bits = current_record->first_6bits & 0x1f;
            if (first_bit) { // edge
                swf_shape_record_edge_t *edge = &(current_record->shape.shape_edge);
                if (edge->shape_edge_type == 0) { // curved
                    signed cx = SCALE_X(edge->shape_control_x), cy = SCALE_Y(edge->shape_control_y);
                    signed ax = SCALE_X(edge->shape_anchor_x), ay = SCALE_Y(edge->shape_anchor_y);
                    if ((bitstream_need_bits_signed(cx - pen_x) > 17) ||
                        (bitstream_need_bits_signed(cy - pen_y) > 17) ||
                        (bitstream_need_bits_signed(ax - cx) > 17) ||
                        (bitstream_need_bits_signed(ay - cy) > 17)) {
                        fprintf(stderr, "swf_shape_record_edge_apply_factor: curve too long for an edge record\n");
                        return 1;
                    }
                    if (pass) {
                        edge->shape_control_x = cx; edge->shape_control_y = cy;
                        edge->shape_anchor_x = ax; edge->shape_anchor_y = ay;
                    }
                    pen_x = ax; pen_y = ay;
                } else { // straight
                    signed x = SCALE_X(edge->shape_x), y = SCALE_Y(edge->shape_y);
                    if ((bitstream_need_bits_signed(x - pen_x) > 17) ||
                        (bitstream_need_bits_signed(y - pen_y) > 17)) {
                        fprintf(stderr, "swf_shape_record_edge_apply_factor: line too long for an edge record\n");
                        return 1;
                    }
                    if (pass) {
                        edge->shape_x = x; edge->shape_y = y;
                    }
                    pen_x = x; pen_y = y;
                }
            } else if (next_5bits) { // setup
                swf_shape_record_setup_t *setup = &(current_record->shape.shape_setup);
                if (setup->shape_has_move_to) { // otherwise the move fields are unused
                    signed x = SCALE_X(setup->shape_move_x), y = SCALE_Y(setup->shape_move_y);
                    if (pass) {
                        setup->shape_move_x = x; setup->shape_move_y = y;
                    }
                    pen_x = x; pen_y = y;
                }
            } else { // end
                break;
            }
        }
    }
#undef SCALE_X
#undef SCALE_Y
    return 0;
}
