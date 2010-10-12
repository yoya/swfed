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
} swf_object_t;


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
                                               double radian,
                                               signed int trans_x,
                                               signed int trans_y);
#endif /* __SWF_OBJECT_H__ */
