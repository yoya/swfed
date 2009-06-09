#include <stdio.h>
#include "bitstream.h"
#include "swf_shape_record.h"

int
swf_shape_record_parse(bitstream_t *bs, swf_shape_record_t *shape_record,
                       swf_tag_t *tag, swf_styles_count_t *count) {
    int first_bit, next_5bits;
    swf_shape_record_t *current = shape_record;
    int limit;
    for (limit = 1; current ; limit ++) {
        current->next = NULL; // fail safe
        int result = bitstream_getbits(bs, 6);
        if (result == -1) {
            fprintf(stderr, "swf_shape_record_parse: bitstream_getbits failed at L%d\n", __LINE__);
            return 1;
        }
        shape_record->shape.first_6bits = result;
        first_bit = (shape_record->shape.first_6bits >> 5) & 1;
        next_5bits = shape_record->shape.first_6bits & 0x1f;
        bitstream_incrpos(bs, 0, -6); // 6 bit back
        if ((first_bit == 0) && (next_5bits == 0)) {
            swf_shape_record_end_parse(bs, &(current->shape.shape_end));
            current->next = NULL;
            break;
        } if (first_bit == 0) {
            swf_shape_record_setup_parse(bs, &(current->shape.shape_setup),
                                         tag, count);
        } else {
            swf_shape_record_edge_parse(bs, &(current->shape.shape_edge));
        }
        if (10000 <= limit) { // XXX 10000???
            current->next = NULL;
            fprintf(stderr, "swf_shape_record_parse: limit(%d) over\n", limit);
            return 1;
        }
        current->next = calloc(1, sizeof(swf_shape_record_t));
        current = current->next;
    }
    return 0;
}

int
swf_shape_record_build(bitstream_t *bs, swf_shape_record_t *shape_record,
                       swf_tag_t *tag, swf_styles_count_t *count) {
    int first_bit, next_5bits;
    swf_shape_record_t *current = shape_record;
    while (current) {
        first_bit = (shape_record->shape.first_6bits >> 5) & 1;
        next_5bits = shape_record->shape.first_6bits & 0x1f;
        if ((first_bit == 0) && (next_5bits == 0)) {
            swf_shape_record_end_build(bs, &(current->shape.shape_end));
            break;
        } if (first_bit == 0) {
            swf_shape_record_setup_build(bs, &(current->shape.shape_setup),
                                         tag, count);
        } else {
            swf_shape_record_edge_build(bs, &(current->shape.shape_edge));
        }
        current = current->next;
    }
    return 0;
}

int
swf_shape_record_print(swf_shape_record_t *shape_record, int indent_depth,
                       swf_tag_t *tag, swf_styles_count_t *count) {
    int first_bit, next_5bits;
    swf_shape_record_t *current = shape_record;
    int i;
    for (i = 0 ; current ; i++) {
        first_bit = (shape_record->shape.first_6bits >> 5) & 1;
        next_5bits = shape_record->shape.first_6bits & 0x1f;
        print_indent(indent_depth);
        printf("shape_record [%d]\n", i);
        if ((first_bit == 0) && (next_5bits == 0)) {
            swf_shape_record_end_print(&(current->shape.shape_end),
                                       indent_depth);
            break;
        } if (first_bit == 0) {
            swf_shape_record_setup_print(&(current->shape.shape_setup),
                                         indent_depth, tag, count);
        } else {
            swf_shape_record_edge_print(&(current->shape.shape_edge),
                                        indent_depth);
        }
        current = current->next;
    }
    return 0;
}

int
swf_shape_record_delete(swf_shape_record_t *shape_record) {
    swf_shape_record_t *current, *next;
    current = shape_record->next;
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
    return 0;
}

