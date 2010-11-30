/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_TAG_ACTION__H__
#define __SWF_TAG_ACTION__H__

#include "swf_tag.h"
#include "swf_action.h"

typedef struct swf_tag_action_detail_ {
    unsigned short  action_sprite;
    unsigned char  *action_record;
    unsigned long   action_record_len;
} swf_tag_action_detail_t;

extern swf_tag_detail_handler_t *swf_tag_action_detail_handler(void);

extern void *swf_tag_action_create_detail(void);
extern int swf_tag_action_input_detail(swf_tag_t *tag,
                                       struct swf_object_ *swf);
extern int swf_tag_action_identity_detail(swf_tag_t *tag, int id);
extern unsigned char *swf_tag_action_output_detail(swf_tag_t *tag,
                                                   unsigned long *length,
                                                   struct swf_object_ *swf);
extern void swf_tag_action_print_detail(swf_tag_t *tag,
                                        struct swf_object_ *swf,
                                        int indent_depth);
extern void swf_tag_action_destroy_detail(swf_tag_t *tag);

extern int swf_tag_action_create_setvaribles(swf_tag_t *tag, y_keyvalue_t *kv);

#endif /* __SWF_TAG_ACTION__H__ */
