#include <stdio.h>
#include "bitstream.h"
#include "swf_shape_record_edge.h"

int
swf_shape_record_edge_parse(bitstream_t *bs,
                            swf_shape_record_edge_t *shape_record_edge) {
    unsigned int shape_coord_real_size;
    shape_record_edge->shape_record_type = bitstream_getbit(bs);
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
    fprintf(stderr, "swf_shape_record_edge_build: not implemented yet\n");
    return 0;
}

int
swf_shape_record_edge_print(swf_shape_record_edge_t *shape_record_edge,
                            int indent_depth) {
    fprintf(stderr, "swf_shape_record_edge_print: not implemented yet\n");
    return 0;
}
