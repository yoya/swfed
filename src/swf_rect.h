/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_RECT_H__
#define __SWF_RECT_H__

typedef struct swf_rect_ {
    unsigned char size;
    int      x_min;
    int      x_max;
    int      y_min;
    int      y_max;
} swf_rect_t;

extern int swf_rect_parse(bitstream_t *bs, swf_rect_t *rect);
extern int swf_rect_build(bitstream_t *bs, swf_rect_t *rect);
extern int swf_rect_print(swf_rect_t *rect);

#endif /* __SWF_RECT_H__ */
