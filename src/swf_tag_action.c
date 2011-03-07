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
    action_detail_handler.input    = swf_tag_action_input_detail;
    action_detail_handler.get_cid  = NULL;
    action_detail_handler.replace_cid  = NULL;
    action_detail_handler.output   = swf_tag_action_output_detail;
    action_detail_handler.print    = swf_tag_action_print_detail;
    action_detail_handler.destroy  = swf_tag_action_destroy_detail;
    return &action_detail_handler;
}

void *
swf_tag_action_create_detail(void) {
    swf_tag_action_detail_t *swf_tag_action;
    swf_tag_action = calloc(sizeof(*swf_tag_action), 1);
    if (swf_tag_action == NULL) {
        fprintf(stderr, "ERROR: swf_tag_action_create_detail: can't calloc\n");
        return NULL;
    }
    swf_tag_action->action_sprite = 0;
    swf_tag_action->action_record = NULL;
    swf_tag_action->action_record_len = 0;
    return swf_tag_action;
}

int
swf_tag_action_input_detail(swf_tag_t *tag, struct swf_object_ *swf) {
    swf_tag_action_detail_t *swf_tag_action = tag->detail;
    unsigned char *data  = tag->data;
    unsigned long length = tag->length;
    bitstream_t *bs;
    unsigned long pos, len;
    (void) swf;
    if (swf_tag_action == NULL) {
        fprintf(stderr, "ERROR: swf_tag_action_create_detail: swf_tag_action == NULL\n");
        return 1;
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
    return 0;
}

unsigned char *
swf_tag_action_output_detail(swf_tag_t *tag, unsigned long *length,
                             struct swf_object_ *swf) {
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
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
swf_tag_action_print_detail(swf_tag_t *tag,
                            struct swf_object_ *swf, int indent_depth) {
    bitstream_t *bs;
    swf_action_list_t *action_list;
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    (void) swf;
    print_indent(indent_depth);
    if (tag->tag == 59) { // DoInitAction
        printf("action_sprite=%d  ", swf_tag_action->action_sprite);
    }
    printf("action_record =\n");
    bs = bitstream_open();
    bitstream_input(bs, swf_tag_action->action_record,
                    swf_tag_action->action_record_len);
    action_list = swf_action_list_create(bs);
    bitstream_close(bs);
    swf_action_list_print(action_list, indent_depth + 1);
    swf_action_list_destroy(action_list);
    return ;
}

void
swf_tag_action_destroy_detail(swf_tag_t *tag) {
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    if (swf_tag_action) {
        free(swf_tag_action->action_record);
        swf_tag_action->action_record = NULL;
        free(swf_tag_action);
        tag->detail = NULL;
    }
    return ;
}

int
swf_tag_action_create_setvaribles(swf_tag_t *tag, y_keyvalue_t *kv) {
    bitstream_t *bs;
    char *key, *value;
    int key_len, value_len;
    unsigned long data_len;
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    swf_tag_action->action_sprite = 0;
    bs = bitstream_open();

    y_keyvalue_rewind(kv);
    while (y_keyvalue_next(kv)) {
        key   = y_keyvalue_get_currentkey(kv, &key_len);
        value = y_keyvalue_get_currentvalue(kv, &value_len);
        bitstream_putbyte(bs, 0x96); // Push Data
        bitstream_putbytesLE(bs, key_len + 2 , 2);
        bitstream_putbyte(bs, 0);
        bitstream_putstring(bs, (unsigned char*) key, key_len);
        bitstream_putbyte(bs, 0);
        bitstream_putbyte(bs, 0x96); // Push Data
        bitstream_putbytesLE(bs, value_len + 2 , 2);
        bitstream_putbyte(bs, 0);
        bitstream_putstring(bs, (unsigned char*) value, value_len);
        bitstream_putbyte(bs, 0);
        bitstream_putbyte(bs, 0x1d); // Set Variable
    }
    bitstream_putbyte(bs, 0); // End
    if (swf_tag_action->action_record) {
        free(swf_tag_action->action_record);
    }
    swf_tag_action->action_record = bitstream_steal(bs, &data_len);
    swf_tag_action->action_record_len = data_len;
    bitstream_close(bs);
    return 0;
}

int
swf_tag_action_put_setvaribles(swf_tag_t *tag, y_keyvalue_t *kv) {
    bitstream_t *bs;
    char *key, *value;
    int key_len, value_len;
    unsigned long data_len;
    swf_tag_action_detail_t *swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    swf_tag_action->action_sprite = 0;
    bs = bitstream_open();
    y_keyvalue_rewind(kv);
    while (y_keyvalue_next(kv)) {
        key   = y_keyvalue_get_currentkey(kv, &key_len);
        value = y_keyvalue_get_currentvalue(kv, &value_len);
        bitstream_putbyte(bs, 0x96); // Push Data
        bitstream_putbytesLE(bs, key_len + 2 , 2);
        bitstream_putbyte(bs, 0);
        bitstream_putstring(bs, (unsigned char *)key, key_len);
        bitstream_putbyte(bs, 0);
        bitstream_putbyte(bs, 0x96); // Push Data
        bitstream_putbytesLE(bs, value_len + 2 , 2);
        bitstream_putbyte(bs, 0);
        bitstream_putstring(bs, (unsigned char *)value, value_len);
        bitstream_putbyte(bs, 0);
        bitstream_putbyte(bs, 0x1d); // Set Variable
    }
    if (swf_tag_action->action_record) {
        bitstream_putstring(bs, swf_tag_action->action_record, swf_tag_action->action_record_len);
        free(swf_tag_action->action_record);
    }
    swf_tag_action->action_record = bitstream_steal(bs, &data_len);
    swf_tag_action->action_record_len = data_len;
    bitstream_close(bs);
    return 0;
}
