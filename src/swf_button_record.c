/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include "bitstream.h"
#include "swf_button_record.h"

int
swf_button_record_parse(bitstream_t *bs, swf_button_record_t *button_record) {
    bitstream_getbits(bs, 2); // reserved bits, always 0
    //
}

int
swf_button_record_build(bitstream_t *bs, swf_button_record_t *button_record) {
    ;
}

void
swf_button_record_destroy(swf_button_record_t *button_record) {
    ;
}

int
swf_button_record_print(swf_button_record_t *button_record, int indent_depth) {
    ;
}


