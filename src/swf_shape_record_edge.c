#include <stdio.h>
#include "bitstream.h"
#include "swf_shape_record_edge.h"

int
swf_shape_record_edge_parse(bitstream_t *bs,
                            swf_shape_record_edge_t *shape_record_edge) {
    int result;
    unsigned int shape_coord_real_size;
    result = bitstream_getbit(bs);
    if (result == -1) {
        return 1;
    }
    shape_record_edge->shape_record_type = result;
    shape_record_edge->shape_edge_type   = bitstream_getbit(bs);
    shape_record_edge->shape_coord_size  = bitstream_getbits(bs, 4);
    shape_coord_real_size = shape_record_edge->shape_coord_size + 2;
    if (shape_record_edge->shape_edge_type == 0) {
        shape_record_edge->shape_control_delta_x = bitstream_getbits(bs, shape_coord_real_size);
        shape_record_edge->shape_control_delta_y = bitstream_getbits(bs, shape_coord_real_size);
        shape_record_edge->shape_anchor_delta_x = bitstream_getbits(bs, shape_coord_real_size);
        shape_record_edge->shape_anchor_delta_y = bitstream_getbits(bs, shape_coord_real_size);
    } else {
        shape_record_edge->shape_line_has_x_and_y = bitstream_getbit(bs);
        if (shape_record_edge->shape_line_has_x_and_y == 1) {
            shape_record_edge->shape_delta_x = bitstream_getbits(bs, shape_coord_real_size);
            shape_record_edge->shape_delta_y = bitstream_getbits(bs, shape_coord_real_size);
        } else {
            shape_record_edge->shape_line_has_x_or_y = bitstream_getbit(bs);
            if (shape_record_edge->shape_line_has_x_or_y == 0) {
                shape_record_edge->shape_delta_x = bitstream_getbits(bs, shape_coord_real_size);
            } else {
                shape_record_edge->shape_delta_y = bitstream_getbits(bs, shape_coord_real_size);
            }
        }
    }
    return 0;
}

int
swf_shape_record_edge_build(bitstream_t *bs,
                            swf_shape_record_edge_t *shape_record_edge) {
//    int result;
    unsigned int shape_coord_real_size;
    bitstream_putbit(bs, shape_record_edge->shape_record_type);
    bitstream_putbit(bs, shape_record_edge->shape_edge_type);
    bitstream_putbits(bs, shape_record_edge->shape_coord_size, 4);
    shape_coord_real_size = shape_record_edge->shape_coord_size + 2;
    if (shape_record_edge->shape_edge_type == 0) {
        bitstream_putbits(bs, shape_record_edge->shape_control_delta_x, shape_coord_real_size);
        bitstream_putbits(bs, shape_record_edge->shape_control_delta_y, shape_coord_real_size);
        bitstream_putbits(bs, shape_record_edge->shape_anchor_delta_x, shape_coord_real_size);
        bitstream_putbits(bs, shape_record_edge->shape_anchor_delta_y, shape_coord_real_size);
    } else {
        bitstream_putbit(bs, shape_record_edge->shape_line_has_x_and_y);
        if (shape_record_edge->shape_line_has_x_and_y == 1) {
            bitstream_putbits(bs, shape_record_edge->shape_delta_x, shape_coord_real_size);
            bitstream_putbits(bs, shape_record_edge->shape_delta_y, shape_coord_real_size);
        } else {
            bitstream_putbit(bs, shape_record_edge->shape_line_has_x_or_y);
            if (shape_record_edge->shape_line_has_x_or_y == 0) {
                bitstream_putbits(bs, shape_record_edge->shape_delta_x, shape_coord_real_size);
            } else {
                bitstream_putbits(bs, shape_record_edge->shape_delta_y, shape_coord_real_size);
            }
        }
    }
    return 0;
}

int
swf_shape_record_edge_print(swf_shape_record_edge_t *shape_record_edge,
                            int indent_depth) {
    print_indent(indent_depth);
    printf("shape_record_type=%d  shape_edge_type=%d  shape_coord_size=%d+2\n", shape_record_edge->shape_record_type,
           shape_record_edge->shape_edge_type ,
           shape_record_edge->shape_coord_size);
    if (shape_record_edge->shape_edge_type == 0) {
        print_indent(indent_depth);
        printf("shape_control_delta_(x,y)=(%d,%d)  shape_anchor_delta_(x,y)=(%d,%d)\n",
               shape_record_edge->shape_control_delta_x,
               shape_record_edge->shape_control_delta_y,
               shape_record_edge->shape_anchor_delta_x,
               shape_record_edge->shape_anchor_delta_y);
    } else {
        print_indent(indent_depth);
        printf("shape_line_has_x_and_y=%d\n",
               shape_record_edge->shape_line_has_x_and_y);
        if (shape_record_edge->shape_line_has_x_and_y == 1) {
            print_indent(indent_depth);
            printf("shape_delta_(x,y)=(%d,%d)\n",
                   shape_record_edge->shape_delta_x,
                   shape_record_edge->shape_delta_y);
        } else {
            print_indent(indent_depth);
            printf("shape_line_has_x_or_y=%d\n",
                   shape_record_edge->shape_line_has_x_or_y);
            if (shape_record_edge->shape_line_has_x_or_y == 0) {
                print_indent(indent_depth);
                printf("shape_delta_x=%d\n",
                       shape_record_edge->shape_delta_x);
            } else {
                print_indent(indent_depth);
                printf("shape_delta_y=%d\n",
                       shape_record_edge->shape_delta_y);
            }
        }
    }
    return 0;
}
