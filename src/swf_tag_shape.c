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
    swf_tag_t **_tag;
    if (swf_tag_shape == NULL) {
        fprintf(stderr, "ERROR: swf_tag_shape_input_detail: swf_tag_shape == NULL\n");
        return 1;
    }
    bs = bitstream_open();
    bitstream_input(bs, data, length);
    swf_tag_shape->shape_id = bitstream_getbytesLE(bs, 2);
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
    swf_tag_t *_tag;
    *length = 0;
    bs = bitstream_open();
    bitstream_putbytesLE(bs, swf_tag_shape->shape_id, 2);
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

void
swf_tag_shape_print_detail(swf_tag_t *tag,
                          struct swf_object_ *swf) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    int i;
    swf_tag_t *_tag;
    printf("\tshape_id=%d\n", swf_tag_shape->shape_id);
    return ;
}

void
swf_tag_shape_destroy_detail(swf_tag_t *tag) {
    swf_tag_shape_detail_t *swf_tag_shape = (swf_tag_shape_detail_t *) tag->detail;
    if (swf_tag_shape) {
        free(swf_tag_shape);
    }
    return ;
}
