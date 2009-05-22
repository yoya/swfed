/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_SHAPE_RECORD_H__
#define __SWF_SHAPE_RECORD_H__

#include "swf_shape_record.h"

typedef union swf_shape_record_ {
    int dummy; int dummy2;
//    swf_shape_record_end_t   shape_end;
//    swf_shape_record_setup_t shape_setup;
//    swf_shape_record_edge_t  shape_edge;
} swf_shape_record_t;

extern int swf_shape_record_parse(bitstream_t *bs, swf_shape_record_t *color);
extern int swf_shape_record_build(bitstream_t *bs, swf_shape_record_t *color);
extern int swf_shape_record_print(swf_shape_record_t *color);

#endif /* __SWF_SHAPE_RECORD_H__ */
