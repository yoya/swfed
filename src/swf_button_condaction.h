/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_BUTTON_CONDACTION_H__
#define __SWF_BUTTON_CONDACTION_H__

#include "bitstream.h"
#include "swf_action.h"

typedef struct swf_button_condaction_ {
    // char align;
    int cond_action_size;
    char cond_idle_to_overdown;
    char cond_outdown_to_idle;
    char cond_overdown_to_outdown;
    char cond_overdown_to_overup;
    char cond_overup_to_idle;
    char cond_idle_to_overup;
    //
    int cond_keypress;
    char cond_overdown_to_idle;
    //
    swf_action_list_t *actions;

} swf_button_condaction_t;

extern int swf_button_condaction_parse(bitstream_t *bs, swf_button_condaction_t *button_condaction);
extern int swf_button_condaction_build(bitstream_t *bs, swf_button_condaction_t *button_condaction);
extern int swf_button_condaction_print(swf_button_condaction_t *button_condaction, int indent_depth);
extern void swf_button_condaction_destroy(swf_button_condaction_t *button_condaction);

#endif /* __SWF_BUTTON_CONDACTION_H__ */
