/* 
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strndup */
#include <zlib.h>
#include "bitstream.h"
#include "swf_define.h"
#include "swf_tag_lossless.h"
#include "swf_png.h"

swf_tag_detail_handler_t lossless_detail_handler;

swf_tag_detail_handler_t *swf_tag_lossless_detail_handler(void) {
    lossless_detail_handler.create   = swf_tag_lossless_create_detail;
    lossless_detail_handler.input    = swf_tag_lossless_input_detail;
    lossless_detail_handler.identity = swf_tag_lossless_identity_detail;
    lossless_detail_handler.output   = swf_tag_lossless_output_detail;
    lossless_detail_handler.print    = swf_tag_lossless_print_detail;
    lossless_detail_handler.destroy  = swf_tag_lossless_destroy_detail;
    return &lossless_detail_handler;
}

void *
swf_tag_lossless_create_detail(void) {
    swf_tag_lossless_detail_t *swf_tag_lossless;
    swf_tag_lossless = calloc(sizeof(*swf_tag_lossless), 1);
    if (swf_tag_lossless == NULL) {
        fprintf(stderr, "swf_tag_lossless_create_detail: can't calloc swf_tag_lossless\n");
        return NULL;
    }
    return swf_tag_lossless;
}

int
swf_tag_lossless_input_detail(unsigned char *data,
                               unsigned long length,
                               swf_tag_t *tag,
                               struct swf_object_ *swf) {
    swf_tag_lossless_detail_t *swf_tag_lossless;
    bitstream_t *bs, *bs2;
    (void) swf;
    unsigned long i;
    unsigned char *tmp_buff, *old_buff_ref;
    unsigned long origsize, old_size, offset;
    int result;
    swf_tag_lossless = tag->detail;
    if (swf_tag_lossless == NULL) {
        fprintf(stderr, "swf_tag_lossless_input_detail: swf_tag_lossless == NULL\n");
        return 1;
    }

    bs = bitstream_open();
    bitstream_input(bs, data, length);
    swf_tag_lossless->image_id = bitstream_getbytesLE(bs, 2);
    swf_tag_lossless->format = bitstream_getbyte(bs);
    swf_tag_lossless->width = bitstream_getbytesLE(bs, 2);
    swf_tag_lossless->height = bitstream_getbytesLE(bs, 2);
    if (swf_tag_lossless->format == 3) {
        unsigned long indices_len;
        int bytes_per_color;
        swf_tag_lossless->colormap_count = bitstream_getbyte(bs) + 1;
        indices_len = ((swf_tag_lossless->width + 3) & -4) * swf_tag_lossless->height;
        if (tag->tag == 20) { // Lossless => rgb (3 bytes)
            bytes_per_color = 3;
        } else { // Lossless2 => rgba (4 bytes)
            bytes_per_color = 4;
        }
        origsize = bytes_per_color * swf_tag_lossless->colormap_count + indices_len;
        tmp_buff = malloc(origsize);
        offset = bitstream_getbytepos(bs);
        old_buff_ref = bitstream_buffer(bs, offset);
        old_size = bitstream_length(bs) - offset;
        result = uncompress(tmp_buff, &origsize, old_buff_ref, old_size);
        if (result != Z_OK) {
            if (result == Z_MEM_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_MEM_ERROR: can't malloc at line(%d)\n", __LINE__);
            } else if (result == Z_BUF_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_BUF_ERROR: not enough buff size at line(%d)\n", __LINE__);
            } else if (result == Z_DATA_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_DATA_ERROR: corrupted or imcomplete data at line(%d)\n", __LINE__);
            } else {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: failed byunknown reason (%d) at line(%d)\n", result, __LINE__);
            }
            free(tmp_buff);
            bitstream_close(bs);
            return 1;
        }
        if (indices_len != origsize - bytes_per_color * swf_tag_lossless->colormap_count) {
            fprintf(stderr, "swf_tag_lossless_create_detail: indices_len(%lu) != origsize(%lu) - %d * swf_tag_lossless->colormap_count(%d) at line(%d)\n",
                    indices_len, origsize, bytes_per_color,
                    swf_tag_lossless->colormap_count, __LINE__);
            free(tmp_buff);
            bitstream_close(bs);
            return 1;
        }
        bs2 = bitstream_open();
        bitstream_input(bs2, tmp_buff, origsize);
        if (tag->tag == 20) { // Lossless
            swf_tag_lossless->colormap = malloc(sizeof(swf_rgb_t) * swf_tag_lossless->colormap_count);
            for (i=0 ; i < swf_tag_lossless->colormap_count ; i++) {
                swf_rgb_t *rgb = swf_tag_lossless->colormap + i;
                swf_rgb_parse(bs2, rgb);
            }
        } else { // tag == 36 (Lossless2)
            swf_tag_lossless->colormap2 = malloc(sizeof(swf_rgba_t) * swf_tag_lossless->colormap_count);
            for (i=0; i<swf_tag_lossless->colormap_count; i++) {
                swf_rgba_t *rgba = swf_tag_lossless->colormap2 + i;
                swf_rgba_parse(bs2, rgba);
            }
        }
        swf_tag_lossless->indices = malloc(indices_len);
        bitstream_getstring(bs2, swf_tag_lossless->indices, indices_len);
        bitstream_close(bs2);
        free(tmp_buff);
    } else { // format != 3
        unsigned long bitmap_count;
        bitmap_count = swf_tag_lossless->width * swf_tag_lossless->height;
        origsize = 4 * bitmap_count; // xrgb or argb (4 bytes)
        tmp_buff = malloc(origsize);
        offset = bitstream_getbytepos(bs);
        old_buff_ref = bitstream_buffer(bs, offset);
        old_size = bitstream_length(bs) - offset;
        result = uncompress(tmp_buff, &origsize, old_buff_ref, old_size);
        if (result != Z_OK) {
            if (result == Z_MEM_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_MEM_ERROR: can't malloc (origsize=%lu, old_size=%lu) at line(%d)\n",
                        origsize, old_size, __LINE__);
            } else if (result == Z_BUF_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_BUF_ERROR: not enough buff size(origsize=%lu, old_size=%lu) at line(%d)\n",
                        origsize, old_size, __LINE__);
            } else if (result == Z_DATA_ERROR) {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: Z_DATA_ERROR: corrupted or imcomplete data at line(%d)\n", __LINE__);
            } else {
                fprintf(stderr, "swf_tag_lossless_create_detail: uncompress: failed byunknown reason (%d) at line(%d)\n", result, __LINE__);
            }
            free(tmp_buff);
            bitstream_close(bs);
            return 1;
        }
        bs2 = bitstream_open();
        bitstream_input(bs2, tmp_buff, origsize);
        if (tag->tag == 20) { // Lossless
            swf_tag_lossless->bitmap = malloc(bitmap_count * sizeof(swf_xrgb_t));
            for (i=0; i<bitmap_count; i++) {
                swf_xrgb_t *xrgb = swf_tag_lossless->bitmap + i;
                swf_xrgb_parse(bs2, xrgb);
            }
        } else { // tag == 36 (Lossless2)
            swf_tag_lossless->bitmap2 = malloc(bitmap_count * sizeof(swf_argb_t));
            for (i=0; i<bitmap_count; i++) {
                swf_argb_t *argb = swf_tag_lossless->bitmap2 + i;
                swf_argb_parse(bs2, argb);
            }
        }
        bitstream_close(bs2);
        free(tmp_buff);
    }
    bitstream_close(bs);
    return 0;

}

int
swf_tag_lossless_identity_detail(unsigned char *data, int id, swf_tag_t *tag) {
    int image_id;
    if (tag->detail) {
        swf_tag_lossless_detail_t *swf_tag_lossless = (swf_tag_lossless_detail_t *) tag->detail;
        if (swf_tag_lossless->image_id == id) {
            return 0;
        }        
        return 1;
    }
    if (data == NULL) {
        fprintf(stderr, "swf_tag_lossless_identity_detail: data==NULL at line(%d)\n", __LINE__);
        return 1;
    }
    image_id = GetUShortLE(data);
    if (id == image_id) {
        return 0;
    }        
    return 1;
}

unsigned char *
swf_tag_lossless_output_detail(void *detail, unsigned long *length,
                               swf_tag_t *tag,
                               struct swf_object_ *swf) {
    swf_tag_lossless_detail_t *swf_tag_lossless = (swf_tag_lossless_detail_t *) detail;
    bitstream_t *bs, *bs2;
    unsigned char *data;
    unsigned long i;
    unsigned char *tmp_buff, *old_buff_ref;
    unsigned long compsize, old_size;
    (void) swf;
    *length = 0;
    bs = bitstream_open();
    bitstream_putbytesLE(bs, swf_tag_lossless->image_id, 2);
    bitstream_putbyte(bs, swf_tag_lossless->format);
    bitstream_putbytesLE(bs, swf_tag_lossless->width, 2);
    bitstream_putbytesLE(bs, swf_tag_lossless->height, 2);
    if (swf_tag_lossless->format == 3) {
        unsigned long indices_len;
        bitstream_putbyte(bs, swf_tag_lossless->colormap_count - 1); /* XXX */
        bs2 = bitstream_open();
        if (tag->tag == 20) { // Lossless
            for (i=0; i<swf_tag_lossless->colormap_count; i++) {
                swf_rgb_t *rgb = swf_tag_lossless->colormap + i;
                swf_rgb_build(bs2, rgb);
            }
        } else { // tag == 36 (Lossless2)
            for (i=0; i<swf_tag_lossless->colormap_count; i++) {
                swf_rgba_t *rgba = swf_tag_lossless->colormap2 + i;
                swf_rgba_build(bs2, rgba);
            }
        }
        indices_len = ((swf_tag_lossless->width + 3) & -4) * swf_tag_lossless->height;
        bitstream_putstring(bs2, swf_tag_lossless->indices,
                            indices_len);
        old_buff_ref = bitstream_buffer(bs2, 0);
        old_size = bitstream_length(bs2);
        tmp_buff = malloc(old_size); // too enough size
        compress(tmp_buff, &compsize, old_buff_ref, old_size);
        bitstream_putstring(bs, tmp_buff, compsize);
        bitstream_close(bs2);
        free(tmp_buff);
    } else {
        unsigned long bitmap_size;
        bs2 = bitstream_open();
        if (tag->tag == 20) { // Lossless
            bitmap_size = swf_tag_lossless->width * swf_tag_lossless->height;
            for (i=0; i<bitmap_size; i++) {
                swf_xrgb_t *xrgb = swf_tag_lossless->bitmap + i;
                swf_xrgb_build(bs2, xrgb);
            }
        } else { // tag == 36 (Lossless2)
            bitmap_size = swf_tag_lossless->width * swf_tag_lossless->height;
            for (i=0; i<bitmap_size; i++) {
                swf_argb_t *argb = swf_tag_lossless->bitmap2 + i;
                swf_argb_build(bs2, argb);
            }
        }
        old_buff_ref = bitstream_buffer(bs2, 0);
        old_size = bitstream_length(bs2);
        compsize = old_size; // too enough size
        tmp_buff = malloc(compsize);
        compress(tmp_buff, &compsize, old_buff_ref, old_size);
        bitstream_putstring(bs, tmp_buff, compsize);
        bitstream_close(bs2);
        free(tmp_buff);
    }
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

void
swf_tag_lossless_print_detail(void *detail,
                              swf_tag_t *tag,
                              struct swf_object_ *swf) {
    swf_tag_lossless_detail_t *swf_tag_lossless = (swf_tag_lossless_detail_t *) detail;
    (void) tag;
    (void) swf;
    if (detail == NULL) {
        fprintf(stderr, "swf_tag_lossless_print_detail: detail == NULL\n");
        return ;
    }
    printf("\timage_id=%d  format=%d  width=%u  height=%u\n",
           swf_tag_lossless->image_id, swf_tag_lossless->format,
           swf_tag_lossless->width, swf_tag_lossless->height);
    if (swf_tag_lossless->colormap ||
        swf_tag_lossless->colormap2) {
        printf("\tcolormap_count=%d",
               swf_tag_lossless->colormap_count);
        if (swf_tag_lossless->colormap) {
            printf("  rgb colormap exists");
        } else {
            printf("  rgba colormap exists");
        }
        if (swf_tag_lossless->indices) {
            printf("  indices exists");
        }
        printf("\n");
    }
    if (swf_tag_lossless->bitmap) {
        printf("\txrgb bitmap exists\n");
    }
    if (swf_tag_lossless->bitmap2) {
        printf("\targb bitmap exists\n");
    }
}

void
swf_tag_lossless_destroy_detail(void *detail) {
    swf_tag_lossless_detail_t *swf_tag_lossless = (swf_tag_lossless_detail_t *) detail;
    if (swf_tag_lossless) {
        free(swf_tag_lossless->colormap);
        free(swf_tag_lossless->colormap2);
        free(swf_tag_lossless->indices);
        free(swf_tag_lossless->bitmap);
        free(swf_tag_lossless->bitmap2);
        swf_tag_lossless->colormap = NULL;
        swf_tag_lossless->colormap2 = NULL;
        swf_tag_lossless->indices = NULL;
        swf_tag_lossless->bitmap = NULL;
        swf_tag_lossless->bitmap2 = NULL;
        free(swf_tag_lossless);
    }
    return ;
}
    
unsigned char *swf_tag_lossless_get_png_data(void *detail,
                                          unsigned long *length,
                                             int image_id,
                                             swf_tag_t *tag) {
    swf_tag_lossless_detail_t *swf_tag_lossless;
    unsigned char *data;
    *length = 0;
    void *index_data = NULL;
    void *image_data = NULL;
    if (detail == NULL) {
        fprintf(stderr, "swf_tag_lossless_get_lossless_data: detail == NULL at line(%d)\n", __LINE__);
    }
    swf_tag_lossless = (swf_tag_lossless_detail_t *) detail;
    if (swf_tag_lossless->image_id != image_id) {
        return NULL;
    }
    if ((swf_tag_lossless->format != 3) && (swf_tag_lossless->format != 5)) {
        fprintf(stderr, "swf_tag_lossless_get_lossless_data: format=%d not implemented yet\n",
                swf_tag_lossless->format);
        return NULL;
    }
    if (tag->tag == 20) {
        if (swf_tag_lossless->format == 3) {
            index_data = (void *) swf_tag_lossless->colormap;
            image_data = (void *) swf_tag_lossless->indices;
        } else {
            image_data = (void *) swf_tag_lossless->bitmap;
        }
    } else { // 36
        if (swf_tag_lossless->format == 3) {
            index_data = (void *) swf_tag_lossless->colormap2;
            image_data = (void *) swf_tag_lossless->indices;
        } else {
            image_data = (void *) swf_tag_lossless->bitmap2;
        }
    }
    if (image_data == NULL) {
        fprintf(stderr, "swf_tag_lossless_get_lossless_data: image_data == NULL at line(%d)\n", __LINE__);
        return NULL;
    }
    data = pngconv_lossless2png(image_data,
                                swf_tag_lossless->width,
                                swf_tag_lossless->height,
                                index_data,
                                swf_tag_lossless->colormap_count,
                                tag->tag, swf_tag_lossless->format,
                                length);
    return data;
}

int
swf_tag_lossless_replace_png_data(void *detail, int image_id,
                                  unsigned char *png_data,
                                  unsigned long png_data_len, swf_tag_t *tag) {
    int tag_no, format;
    unsigned short width, height;
    unsigned char *result_data;
    void *colormap = NULL;
    int colormap_count = 0;
    swf_tag_lossless_detail_t *swf_tag_lossless = (swf_tag_lossless_detail_t *) detail;
    if (detail == NULL) {
        fprintf(stderr, "swf_tag_lossless_replace_lossless_data: detail == NULL at line(%d)\n", __LINE__);
        return 1;
    }
    if (swf_tag_lossless->image_id != image_id) {
        return 1;
    }
    result_data = pngconv_png2lossless(png_data, png_data_len,
                                       &tag_no, &format,
                                       &width, &height,
                                       &colormap, &colormap_count);

    if (result_data == NULL) {
        fprintf(stderr, "swf_tag_lossless_replace_lossless_data: pngconv_png2lossless failed at line(%d)\n", __LINE__);
        return 1;
    }
    tag->tag = tag_no;
    swf_tag_lossless->format = format;
    swf_tag_lossless->width  = width;
    swf_tag_lossless->height = height;
    if (format == 3) {
        free(swf_tag_lossless->colormap);
        free(swf_tag_lossless->colormap2);
        free(swf_tag_lossless->indices);
        free(swf_tag_lossless->bitmap);
        free(swf_tag_lossless->bitmap2);
        swf_tag_lossless->colormap = NULL;
        swf_tag_lossless->colormap2 = NULL;
        swf_tag_lossless->indices = NULL;
        swf_tag_lossless->bitmap = NULL;
        swf_tag_lossless->bitmap2 = NULL;
        if (tag_no == 20) {
            swf_tag_lossless->colormap = (swf_rgb_t*) colormap;
        } else if (tag_no == 36) {
            swf_tag_lossless->colormap2 = (swf_rgba_t*) colormap;
        } else {
            fprintf(stderr, "swf_tag_lossless_replace_lossless_data: internal error tag_no(%d) at line(%d).\n", tag_no, __LINE__);
            return 1;
        }
        swf_tag_lossless->colormap_count = colormap_count;
        swf_tag_lossless->indices = (unsigned char *) result_data;
    } else if (format == 5) {
        free(swf_tag_lossless->colormap);
        free(swf_tag_lossless->colormap2);
        free(swf_tag_lossless->indices);
        free(swf_tag_lossless->bitmap);
        free(swf_tag_lossless->bitmap2);
        swf_tag_lossless->colormap = NULL;
        swf_tag_lossless->colormap2 = NULL;
        swf_tag_lossless->indices = NULL;
        swf_tag_lossless->bitmap = NULL;
        swf_tag_lossless->bitmap2 = NULL;
        if (tag_no == 20) {
            swf_tag_lossless->bitmap = (swf_xrgb_t*) result_data;
        } else if (tag_no == 36) {
            swf_tag_lossless->bitmap2 = (swf_argb_t*) result_data;
        } else {
            fprintf(stderr, "swf_tag_lossless_replace_lossless_data: internal error tag_no(%d) at line(%d).\n", tag_no, __LINE__);
            return 1;
        }
    } else {
        fprintf(stderr, "swf_tag_lossless_replace_lossless_data: format(%d) not implemented yet. at line(%d)\n", format, __LINE__);
        return 1;
    }
    return 0;
}
