/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

extern void *
pngconv_png2lossless(unsigned char *png_data, unsigned long png_data_len,
                     int *tag, int *format,
                     unsigned short *width, unsigned short *height,
                     void **colormap, int *colormap_count);

extern unsigned char *
pngconv_lossless2png(void *image_data,
                     unsigned short width, unsigned short height,
                     void *index_data,
                     unsigned short index_data_count,
                     int tag, int format,
                     unsigned long *length);
