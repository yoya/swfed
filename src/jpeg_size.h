/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/


#ifndef __JPEG_SIZE_H__
#define __JPEG_SIZE_H__

extern int jpeg_size_segment(jpeg_segment_t *jpeg_seg, int *width, int *height);
extern int jpeg_size(unsigned char *data, unsigned long data_len,
                     int *width, int *height);

#endif /* __JPEG_SEGMENT_H__ */
