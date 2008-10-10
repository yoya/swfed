/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_tag_action.h"
#include "swf_object.h"

swf_tag_detail_handler_t action_detail_handler;

swf_tag_detail_handler_t *
swf_tag_action_detail_handler(void) {
    action_detail_handler.create   = swf_tag_action_create_detail;
    action_detail_handler.identity = swf_tag_action_identity_detail;
    action_detail_handler.output   = swf_tag_action_output_detail;
    action_detail_handler.print    = swf_tag_action_print_detail;
    action_detail_handler.destroy  = swf_tag_action_destroy_detail;
    return &action_detail_handler;
}

void *
swf_tag_action_create_detail(unsigned char *data,
                             unsigned long length,
                             swf_tag_t *tag,
                             struct swf_object_ *swf) {
    swf_tag_action_detail_t *swf_tag_action;
    bitstream_t *bs;
    unsigned long pos, len;
    (void) swf;
    swf_tag_action = calloc(sizeof(*swf_tag_action), 1);
    if (swf_tag_action == NULL) {
        fprintf(stderr, "ERROR: swf_tag_action_create_detail: can't calloc\n");
        return NULL;
    }
    bs = bitstream_open();
    bitstream_input(bs, data, length);

    if (tag->tag == 59) { // DoInitAction
        swf_tag_action->action_sprite = bitstream_getbytesLE(bs, 2);
    } else {              // DoAction
        swf_tag_action->action_sprite = 0; // fail safe
    }
    pos = bitstream_getbytepos(bs);
    len = bitstream_length(bs) - pos;
    swf_tag_action->action_record = bitstream_output_sub(bs, pos, len);
    swf_tag_action->action_record_len = len;
    bitstream_close(bs);
    return (void *) swf_tag_action;
}

int swf_tag_action_identity_detail(unsigned char *data, int id,
                                   swf_tag_t *tag) {
    (void) data;
    (void) id;
    (void) tag;
    return 1;
}

unsigned char *
swf_tag_action_output_detail(void *detail, unsigned long *length,
                             swf_tag_t *tag,
                             struct swf_object_ *swf) {
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) detail;
    bitstream_t *bs;
    unsigned char *data;
    *length = 0;

    (void) swf;
    bs = bitstream_open();
    if (tag->tag == 59) { // DoInitAction
        bitstream_putbytesLE(bs, swf_tag_action->action_sprite, 2);
    } else {              // DoAction
        ; // nothing
    }
    bitstream_putstring(bs, swf_tag_action->action_record,
                        swf_tag_action->action_record_len);
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

void
swf_tag_action_print_detail(void *detail,
                            swf_tag_t *tag,
                            struct swf_object_ *swf) {
    bitstream_t *bs;
    swf_action_list_t *action_list;
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) detail;
    (void) swf;
    if (tag->tag == 59) { // DoInitAction
        printf("action_sprite=%d  ", swf_tag_action->action_sprite);
    }
    printf("action_record =\n");
    bs = bitstream_open();
    bitstream_input(bs, swf_tag_action->action_record,
                    swf_tag_action->action_record_len);
    action_list = swf_action_list_create(bs);
    bitstream_close(bs);
    swf_action_list_print(action_list);
    swf_action_list_destroy(action_list);
    return ;
}

void
swf_tag_action_destroy_detail(void *detail) {
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) detail;
    if (swf_tag_action) {
        free(swf_tag_action->action_record);
        swf_tag_action->action_record = NULL;
        free(swf_tag_action);
    }
    return ;
}
