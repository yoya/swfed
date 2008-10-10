/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_TAG_H__
#define __SWF_TAG_H__

#include "bitstream.h"
struct swf_object_; // swf_object.h

typedef struct swf_tag_ {
    int tag;
    unsigned long length;
    int length_longformat;
    //
    unsigned char *data;
    void *detail;
    //
    struct swf_tag_ *next;
} swf_tag_t;

typedef struct swf_tag_detail_handler_ {
    void          * (*create)   (unsigned char *data,
                                 unsigned long length,
                                 swf_tag_t *tag,
                                 struct swf_object_ *swf);
    int             (*identity) (unsigned char *data, int id,
                                 swf_tag_t *tag);
    unsigned char * (*output)   (void *detail, unsigned long *length,
                                 swf_tag_t *tag,
                                 struct swf_object_ *swf);
    void            (*print)    (void *detail,
                                 swf_tag_t *tag,
                                 struct swf_object_ *swf);
    void            (*destroy)  (void *);
} swf_tag_detail_handler_t;

typedef struct swf_tag_info_ {
    int id;
    const char *name;
    swf_tag_detail_handler_t *(*detail_handler)(void);
} swf_tag_info_t;

extern swf_tag_info_t *get_swf_tag_info(int tag_id);

extern swf_tag_t *swf_tag_create(bitstream_t *bs);
extern void swf_tag_destroy(swf_tag_t *tag);
extern int swf_tag_build(bitstream_t *bs, swf_tag_t *tag, struct swf_object_ *swf);
extern void swf_tag_print(swf_tag_t *tag, struct swf_object_ *swf);

/* image */

extern unsigned char *swf_tag_get_jpeg_data(swf_tag_t *tag, unsigned long *length, int image_id, swf_tag_t *tag_jpegtables);
extern unsigned char *swf_tag_get_alpha_data(swf_tag_t *tag, unsigned long *length, int image_id);
extern int swf_tag_replace_jpeg_data(swf_tag_t *tag, int image_id,
                                     unsigned char *jpeg_data,
                                     unsigned long jpeg_data_len,
                                     unsigned char *alpha_data,
                                     unsigned long alpha_data_len);

extern unsigned char *swf_tag_get_png_data(swf_tag_t *tag, unsigned long *length, int image_id);
extern int swf_tag_replace_png_data(swf_tag_t *tag, int image_id,
                                    unsigned char *png_data,
                                    unsigned long png_data_len);
/* sound */
extern unsigned char *swf_tag_get_sound_data(swf_tag_t *tag,
                                             unsigned long *length,
                                             int sound_id);
extern int swf_tag_replace_melo_data(swf_tag_t *tag, int sound_id,
                                     unsigned char *melo_data,
                                     unsigned long melo_data_len);
/* edit text */

extern char *swf_tag_get_edit_string(swf_tag_t *tag,
                                     char *variable_name,
                                     int variable_name_len,
                                     struct swf_object_ *swf);
extern int swf_tag_replace_edit_string(swf_tag_t *tag,
                                       char *variable_name,
                                       int variable_name_len,
                                       char *initial_text,
                                       int initial_text_len,
                                       struct swf_object_ *swf);

extern int swf_tag_create_detail(swf_tag_t *tag, struct swf_object_ *swf);

#endif /* __SWF_TAG_H__ */
