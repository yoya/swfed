/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_BUTTON_RECORD_H__
#define __SWF_BUTTON_RECORD_H__

#include "swf_matrix.h"
#include "swf_cxformwithalpha.h"

typedef struct swf_button_record_ {
    // char align;
    int button_has_blend_mode ;
    char button_has_filter_list : 1;
    char buttun_state_hit_test : 1;
    char button_state_down : 1;
    char button_state_over : 1;
    char button_state_up : 1;
    int character_id;
    int place_depth;
    swf_matrix_t *place_matrix;
    swf_cxformwithalpha_t *cxform_with_alpha;


// not implemented yet. (swf 8 later only)
//    swf_filterlist_t *filter_list;
//    int blend_mode;
} swf_button_record_t;

extern int swf_button_record_parse(bitstream_t *bs, swf_button_record_t *button_record);
extern int swf_button_record_build(bitstream_t *bs, swf_button_record_t *button_record);
extern void swf_button_record_destroy(swf_button_record_t *button_record);
extern int swf_button_record_print(swf_button_record_t *button_record, int indent_depth);

#endif /* __SWF_BUTTON_RECORD_H__ */

