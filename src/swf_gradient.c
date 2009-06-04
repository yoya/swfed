#include <stdio.h>
#include "bitstream.h"
#include "swf_gradient.h"

int
swf_gradient_parse(bitstream_t *bs, swf_gradient_t *gradient,
                   swf_tag_t *tag) {
    int i;
    if (tag->tag == 83) { // DefineShape4
        gradient->spread_mode = bitstream_getbits(bs, 2);
        gradient->interpolation_mode = bitstream_getbits(bs, 2);
        gradient->count = bitstream_getbits(bs, 4);
    } else {
        gradient->pad = bitstream_getbits(bs, 4);
        gradient->count = bitstream_getbits(bs, 4);
    }
    for (i = 0 ; i < gradient->count ; i++) {
        swf_grandient_record_parse(bs, &(gradient->gradient_record[i]));
        gradient->focal_point = bitstream_getbytesLE(bs, 2);
    }
    return 0;
}

int
swf_gradient_build(bitstream_t *bs, swf_gradient_t *gradient,
                   swf_tag_t *tag) {
    return 0;
}

int
swf_gradient_print(swf_gradient_t *gradient, int indent_depth,
                   swf_tag_t *tag) {
    print_indent(indent_depth);
    return 0;
}
