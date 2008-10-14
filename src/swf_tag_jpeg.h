/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#ifndef __SWF_TAG_JPEG__H__
#define __SWF_TAG_JPEG__H__

#include "swf_tag.h"

typedef struct swf_tag_jpeg_detail_ {
    int image_id;
    unsigned char *jpeg_data;
    unsigned long  jpeg_data_len;
    unsigned long  offset_to_alpha;
    unsigned char *alpha_data;
    unsigned long  alpha_data_len;
} swf_tag_jpeg_detail_t;

extern swf_tag_detail_handler_t *swf_tag_jpeg_detail_handler(void);
extern swf_tag_detail_handler_t *swf_tag_jpeg3_detail_handler(void);

extern void *swf_tag_jpeg_create_detail(void);

extern int swf_tag_jpeg_input_detail(unsigned char *data,
                                     unsigned long length,
                                     swf_tag_t *tag,
                                     struct swf_object_ *swf);
extern int swf_tag_jpeg3_input_detail(unsigned char *data,
                                      unsigned long length,
                                      swf_tag_t *tag,
                                      struct swf_object_ *swf);
extern int swf_tag_jpeg_identity_detail(unsigned char *data, int id,
                                        swf_tag_t *tag);
extern void *swf_tag_jpeg3_create_detail(unsigned char *data,
                                         unsigned long length,
                                         swf_tag_t *tag,
                                         struct swf_object_ *swf);
extern unsigned char *swf_tag_jpeg_output_detail(void *detail,
                                                 unsigned long *length,
                                                 swf_tag_t *tag,
                                                 struct swf_object_ *swf);
extern unsigned char *swf_tag_jpeg3_output_detail(void *detail,
                                                  unsigned long *length,
                                                  swf_tag_t *tag,
                                                  struct swf_object_ *swf);
extern void swf_tag_jpeg_print_detail(void *detail,
                                      swf_tag_t *tag,
                                      struct swf_object_ *swf);
extern void swf_tag_jpeg_destroy_detail(void *detail);

extern unsigned char *swf_tag_jpeg_get_jpeg_data(void *detail,
                                                 unsigned long *length,
                                                 int image_id,
                                                 unsigned char *jpeg_table_data,
                                                 unsigned long jpeg_table_data_len);
extern unsigned char *swf_tag_jpeg_get_alpha_data(void *detail, unsigned long *length, int image_id);
extern int swf_tag_jpeg_replace_jpeg_data(void *detail, int image_id,
                                          unsigned char *jpeg_data,
                                          unsigned long jpeg_data_len,
                                          unsigned char *alpha_data,
                                          unsigned long alpha_data_len,
                                          swf_tag_t *tag);

#endif /* __SWF_TAG_JPEG__H__ */
