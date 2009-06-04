#include <stdio.h>
#include "bitstream.h"
#include "swf_gradient_record.h"

int
swf_gradient_record_parse(bitstream_t
                          *bs, swf_gradient_record_t *gradient_record,
                          swf_tag_t *tag) {
    if (tag->tag == 46 || tag->tag == 84) { // DefineMorph || DefineMorphShape2
        gradient_record->position = bitstream_getbyte(bs);
        swf_rgba_parse(bs, &(gradient_record->rgba));
        gradient_record->position_morph = bitstream_getbyte(bs);
        swf_rgba_parse(bs, &(gradient_record->rgba_morph));
    } else if (tag->tag == 32 || tag->tag == 83) {
        // DefineShape3 || DefineShape4
        gradient_record->position = bitstream_getbyte(bs);
        swf_rgba_parse(bs, &(gradient_record->rgba));
    } else {
        gradient_record->position = bitstream_getbyte(bs);
        swf_rgb_parse(bs, &(gradient_record->rgb));
    }
    return 0;
}

int
swf_gradient_record_build(bitstream_t *bs,
                          swf_gradient_record_t *gradient_record,
                          swf_tag_t *tag) {
    return 0;
}

int
swf_gradient_record_print(swf_gradient_record_t *gradient_record,
                          int indent_depth, swf_tag_t *tag) {
    print_indent(indent_depth);
    return 0;
}
