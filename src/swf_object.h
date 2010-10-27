/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_OBJECT_H__
#define __SWF_OBJECT_H__

#include "swf_header.h"
#include "swf_header.h"
#include "swf_tag.h"

typedef struct swf_object_ {
    swf_header_t header;
    swf_header_movie_t header_movie;
    swf_tag_t *tag;
    unsigned adjust_shape_bitmap_mode;
} swf_object_t;

#define SWFED_SHAPE_BITMAP_NONE           0
#define SWFED_SHAPE_BITMAP_MATRIX_RESCALE 1
#define SWFED_SHAPE_BITMAP_RECT_RESIZE    2
#define SWFED_SHAPE_BITMAP_TYPE_TILLED    4

extern swf_object_t *swf_object_open(void);
extern void swf_object_close(swf_object_t *swf);
extern int swf_object_input(swf_object_t *swf, unsigned char *data, unsigned long data_len);
extern unsigned char *swf_object_output(swf_object_t *swf, unsigned long *length);

/* --- */

extern void swf_object_print(swf_object_t *swf);
extern unsigned char *swf_object_get_tagdata(swf_object_t *swf, int tag_seqno,
                                             unsigned long *length);
extern int swf_object_replace_tagdata(swf_object_t *swf, int tag_seqno,
                                      unsigned char *data,
                                      unsigned long *length);

/* --- */

extern int swf_object_adjust_shapebitmap(swf_object_t *swf, unsigned mode);

/* --- */

extern unsigned char *swf_object_get_jpegdata(swf_object_t *swf, unsigned long *length, int image_id);
extern unsigned char *swf_object_get_alphadata(swf_object_t *swf, unsigned long *length, int image_id);
extern int swf_object_replace_jpegdata(swf_object_t *swf, int image_id,
                                       unsigned char *jpeg_data,
                                       unsigned long jpeg_data_len,
                                       unsigned char *alpha_data,
                                       unsigned long alpha_data_len);
extern unsigned char *swf_object_get_pngdata(swf_object_t *swf, unsigned long *length, int image_id);
extern int swf_object_replace_pngdata(swf_object_t *swf, int image_id,
                                      unsigned char *png_data,
                                      unsigned long png_data_len);
extern int swf_object_replace_gifdata(swf_object_t *swf, int image_id,
                                      unsigned char *gif_data,
                                      unsigned long gif_data_len);
extern unsigned char *swf_object_get_sounddata(swf_object_t *swf, unsigned long *length, int sound_id);
extern int swf_object_replace_melodata(swf_object_t *swf, int sound_id,
                                       unsigned char *melo_data,
                                       unsigned long melo_data_len);
extern char *swf_object_get_editstring(swf_object_t *swf,
                                       char *variable_name,
                                       int variable_name_len);
extern int swf_object_replace_editstring(swf_object_t *swf,
                                         char *variable_name,
                                         int variable_name_len,
                                         char *initial_text,
                                         int initial_text_len);
extern unsigned char *swf_object_get_actiondata(swf_object_t *swf, unsigned long *length, int tag_seqno);

extern int swf_object_apply_shapematrix_factor(swf_object_t *swf,int shape_id,
                                               double scale_x, double scale_y,
                                               double rotate_rad,
                                               signed int trans_x,
                                               signed int trans_y);
extern int swf_object_apply_shaperect_factor(swf_object_t *swf,int shape_id,
                                             double scale_x, double scale_y,
                                             signed int trans_x,
                                             signed int trans_y);
extern int swf_object_apply_shapetype_tilled(swf_object_t *swf,int shape_id);

#endif /* __SWF_OBJECT_H__ */
