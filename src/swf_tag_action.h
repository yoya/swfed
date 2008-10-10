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

extern void *swf_tag_action_create_detail(unsigned char *data,
                                          unsigned long length,
                                          swf_tag_t *tag,
                                          struct swf_object_ *swf);
extern int swf_tag_action_identity_detail(unsigned char *data, int id,
                                          swf_tag_t *tag);
extern unsigned char *swf_tag_action_output_detail(void *detail,
                                                   unsigned long *length,
                                                   swf_tag_t *tag,
                                                   struct swf_object_ *swf);
extern void swf_tag_action_print_detail(void *detail,
                                        swf_tag_t *tag,
                                        struct swf_object_ *swf);
extern void swf_tag_action_destroy_detail(void *detail);

#endif /* __SWF_TAG_ACTION__H__ */
