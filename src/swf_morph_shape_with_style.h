/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_MORPH_SHAPE_WITH_STYLE_H__
#define __SWF_MORPH_SHAPE_WITH_STYLE_H__

#include "swf_styles.h"
#include "swf_shape_record.h"
#include "swf_styles_count.h"

typedef struct swf_morph_shape_with_style_ {
    swf_styles_t       styles;
    swf_shape_record_t *shape_records;
    // char align;
    swf_styles_count_t styles_count;
    swf_shape_record_t *shape_records_morph;
    
} swf_morph_shape_with_style_t;

extern int swf_morph_shape_with_style_parse(bitstream_t *bs, swf_morph_shape_with_style_t *color);
extern int swf_morph_shape_with_style_build(bitstream_t *bs, swf_morph_shape_with_style_t *color);
extern int swf_morph_shape_with_style_print(swf_morph_shape_with_style_t *color, int indent_depth);

#endif /* __SWF_MORPH_SHAPE_WITH_STYLE_H__ */
