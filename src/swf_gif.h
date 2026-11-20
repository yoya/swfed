/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_GIF_H__
#define __SWF_GIF_H__

extern void *
gifconv_gif2lossless(unsigned char *gif_data, unsigned long gif_data_len,
                     int *tag, int *format,
                     unsigned short *width, unsigned short *height,
                     void **colormap, int *colormap_count);

#endif /* __SWF_GIF_H__ */
