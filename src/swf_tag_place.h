/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_TAG_PLACE__H__
#define __SWF_TAG_PLACE__H__

#include "swf_matrix.h"
#include "swf_cxform.h"
#include "swf_tag.h"

typedef struct swf_tag_place_detail_ {
    int character_id; //  Shape2 optional.
    int flag_has_clip_action;
    int flag_has_clip_depth;
    int flag_has_name;
    int flag_has_ratio;
    int flag_has_color_transform;
    int flag_has_matrix;
    int flag_has_character;
    int flag_has_movie;
    int depth;
    swf_matrix_t matrix;
    swf_cxform_t color_transform;
    int ratio;
    char *name;
    int clip_depth;
} swf_tag_place_detail_t;

extern swf_tag_detail_handler_t *swf_tag_place_detail_handler(void);

extern void *swf_tag_place_create_detail(void);
extern int swf_tag_place_input_detail(swf_tag_t *tag, struct swf_object_ *swf);
extern unsigned char *swf_tag_place_output_detail(swf_tag_t *tag,
                                                   unsigned long *length,
                                                   struct swf_object_ *swf);
extern void swf_tag_place_print_detail(swf_tag_t *tag,
                                       struct swf_object_ *swf,
                                       int indent_depth);
extern void swf_tag_place_destroy_detail(swf_tag_t *tag);

#endif /* __SWF_TAG_PLACE__H__ */

