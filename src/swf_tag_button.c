/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_button_record.h"
#include "swf_button_condaction.h"
#include "swf_tag.h"
#include "swf_tag_button.h"
#include "swf_object.h"

swf_tag_detail_handler_t button_detail_handler;

swf_tag_detail_handler_t *
swf_tag_button_detail_handler(void) {
    button_detail_handler.create   = swf_tag_button_create_detail;
    button_detail_handler.input    = swf_tag_button_input_detail;
    button_detail_handler.get_cid  = swf_tag_button_get_cid_detail;
    button_detail_handler.replace_cid = swf_tag_button_replace_cid_detail;
    button_detail_handler.output   = swf_tag_button_output_detail;
    button_detail_handler.print    = swf_tag_button_print_detail;
    button_detail_handler.destroy  = swf_tag_button_destroy_detail;
    return &button_detail_handler;
}

void *
swf_tag_button_create_detail(void) {
    swf_tag_button_detail_t *swf_tag_button;
    swf_tag_button = calloc(sizeof(*swf_tag_button), 1);
    if (swf_tag_button == NULL) {
        fprintf(stderr, "ERROR: swf_tag_button_create_detail: can't calloc\n");
        return NULL;
    }
    return swf_tag_button;
}

int
swf_tag_button_input_detail(swf_tag_t *tag, struct swf_object_ *swf) {
    swf_tag_button_detail_t *swf_tag_button = tag->detail;
    unsigned char *data  = tag->data;
    unsigned long length = tag->length;
    bitstream_t *bs;
    int ret;
    (void) swf;
    if (swf_tag_button == NULL) {
        fprintf(stderr, "ERROR: swf_tag_button_input_detail: swf_tag_place == NULL\n");
        return 1;
    }
    bs = bitstream_open();
    bitstream_input(bs, data, length);

    if (tag->code != 34) { // not ButtonObject2
        fprintf(stderr, "swf_tag_button_input_detail: tag->code != 34\n");
        return 1; // failed
    }
    swf_tag_button->button_id = bitstream_getbytesLE(bs, 2);
    bitstream_getbits(bs, 7); // reserved flags : always 0
    swf_tag_button->track_as_menu = bitstream_getbit(bs);
    swf_tag_button->action_offset = bitstream_getbytesLE(bs, 2);
    
    return 0;
}

int
swf_tag_button_get_cid_detail(swf_tag_t *tag) {
    unsigned char *data = tag->data;
    if (tag->detail) {
        swf_tag_button_detail_t *swf_tag_button = (swf_tag_button_detail_t *) tag->detail;
        return swf_tag_button->button_id;
    }
    if (data == NULL) {
        fprintf(stderr, "swf_tag_button_get_cid_detail: data == NULL\n");
        return -1;
    }
    return GetUShortLE(data); // button_id;
}

int swf_tag_button_replace_cid_detail(swf_tag_t *tag, int id) {
    unsigned char *data = tag->data;
    if (tag->detail) {
        swf_tag_button_detail_t *swf_tag_button = (swf_tag_button_detail_t *) tag->detail;
        swf_tag_button->button_id = id;
    }
    if (data) {
        PutUShortLE(data, id);
    }
    return 0; // always 0
}

unsigned char *
swf_tag_button_output_detail(swf_tag_t *tag, unsigned long *length,
                             struct swf_object_ *swf) {
    swf_tag_button_detail_t *swf_tag_button = (swf_tag_button_detail_t *) tag->detail;
}

void
swf_tag_button_print_detail(swf_tag_t *tag,
                           struct swf_object_ *swf, int indent_depth) {
    swf_tag_button_detail_t *swf_tag_button = (swf_tag_button_detail_t *) tag->detail;
    (void) swf;
    print_indent(indent_depth);
    printf("button_id=%d\n", swf_tag_button->button_id);
}

void
swf_tag_button_destroy_detail(swf_tag_t *tag) {
    swf_tag_button_detail_t *swf_tag_button = (swf_tag_button_detail_t *) tag->detail;
    if (swf_tag_button) {
        if (swf_tag_button->button_record) {
            swf_button_record_destroy(swf_tag_button->button_record);
            swf_tag_button->button_record = NULL;
        }
        if (swf_tag_button->actions) {
            swf_button_condaction_destroy(swf_tag_button->actions);
            swf_tag_button->actions = NULL;
        }
        free(swf_tag_button);
        tag->detail = NULL;
    }
    return ;
}
