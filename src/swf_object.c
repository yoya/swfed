/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcmp
#include <zlib.h>
#include "bitstream.h"
#include "swf_define.h"
#include "swf_tag_action.h"
#include "swf_tag_jpeg.h"
#include "swf_tag_lossless.h"
#include "swf_tag_shape.h"
#include "swf_action.h"
#include "swf_object.h"
#include "bitmap_util.h"

swf_object_t *
swf_object_open(void) {
    swf_object_t *swf;
    malloc_debug_start(); /* DEBUG XXX */
    swf = (swf_object_t *) calloc(sizeof(*swf), 1);
    //
    swf->adjust_shape_bitmap_mode = 0;
    return swf;
}

void
swf_object_close(swf_object_t *swf) {
    swf_tag_t *tag, *next_tag;
    if (! swf) {
        return ;
    }
    for (tag=swf->tag ; tag ; tag=next_tag) {
        next_tag = tag->next;
        swf_tag_destroy(tag);
    }
    free(swf);
    malloc_debug_end(); /* DEBUG XXX */
    return ;
}
int
swf_object_input(swf_object_t *swf, unsigned char *data,
                 unsigned long data_len) {
    int result;
    bitstream_t *bs;
    swf_tag_t *tag, *prev_tag, *head_tag;
    bs = bitstream_open();
    bitstream_input(bs, data, data_len);
    result = swf_header_parse(bs, &swf->header);
    if (result) {
        bitstream_close(bs);
        return result;
    }
    if (memcmp(swf->header.magic, "FWS", 3) == 0) {
        ; // OK
    } else if (memcmp(swf->header.magic, "CWS", 3) == 0) {
        int result;
        unsigned char *old_buff_ref, *new_buff;
        unsigned long origsize;
        old_buff_ref = bitstream_buffer(bs, SWF_HEADER_SIZE);
        origsize = swf->header.file_length - SWF_HEADER_SIZE;
        new_buff = malloc(origsize);
        result = uncompress(new_buff, &origsize, old_buff_ref, bs->data_len - SWF_HEADER_SIZE);
        if (result != Z_OK) {
            if (result == Z_MEM_ERROR) {
                fprintf(stderr, "swf_object_input: uncompress Z_MEM_ERROR: can't malloc\n");
            } else if (result == Z_BUF_ERROR) {
                fprintf(stderr, "swf_object_input: uncompress Z_BUF_ERROR: not enough buff size\n");
            } else {
                fprintf(stderr, "swf_object_input: uncompress failed by unknown reason\n");
            }
            free(new_buff);
            bitstream_close(bs);
            return 1; // FAILURE
        }
        bitstream_putstring(bs, new_buff, origsize);
        free(new_buff);
        bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
    } else {
        fprintf(stderr, "swf_object_input: unknown magic %s\n", swf->header.magic);
        bitstream_close(bs);
        return 1; // FAILURE
    }
    result = swf_header_movie_parse(bs, &swf->header_movie);
    if (result) {
        bitstream_close(bs);
        return result;
    }
    head_tag = prev_tag = NULL;
    while(1) {
        long pos;
        pos = bitstream_getbytepos(bs);
        if ((pos == -1) || ((long) swf->header.file_length <= pos)) {
            break;
        }
        tag = swf_tag_create(bs);
	if (tag == NULL) {
	    swf_tag_t *next_tag;
	    for (tag = head_tag ; tag ; tag = next_tag) {
	        next_tag = tag->next;
		swf_tag_destroy(tag);
	    }
	    bitstream_close(bs);
	    return 1; // FAILURE
	}
        if (head_tag == NULL) {
            head_tag = tag;
        } else if (tag) {
            prev_tag->next = tag;
            tag->next = NULL;
        }
	if (tag->tag == 0) { // END Tag
	  break; // SUCCESS
	}
        prev_tag = tag;
    }
    swf->tag = head_tag;
    bitstream_close(bs);
    return 0;
}

unsigned char *
swf_object_output(swf_object_t *swf, unsigned long *length) {
    int result;
    swf_tag_t *tag;
    unsigned char *data;
    bitstream_t *bs = bitstream_open();
    *length = 0;
    result = swf_header_build(bs, &swf->header);
    if (result) {
        bitstream_close(bs);
        return NULL;
    }
    result = swf_header_movie_build(bs, &swf->header_movie);
    if (result) {
        bitstream_close(bs);
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        swf_tag_build(bs, tag, swf);
    }
    swf->header.file_length = bitstream_getbytepos(bs);
    bitstream_setpos(bs, SWF_MAGIC_SIZE, 0);
    bitstream_putbytesLE(bs, swf->header.file_length,
                         SWF_FILE_LENGTH_SIZE);
    if (memcmp(swf->header.magic, "FWS", SWF_MAGIC_SIZE) == 0) {
        ; // OK
    } else if (memcmp(swf->header.magic, "CWS", SWF_MAGIC_SIZE) == 0) {
        int result;
        unsigned long compsize, old_size;
        unsigned char *new_buff, *old_buff_ref;
        bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
        old_buff_ref = bitstream_buffer(bs, SWF_HEADER_SIZE);
        old_size = bs->data_len - SWF_HEADER_SIZE;
        compsize = old_size;
        new_buff = malloc(compsize);
        result = compress(new_buff, &compsize, old_buff_ref, old_size);
        if (result != Z_OK) {
            if (result == Z_MEM_ERROR) {
                fprintf(stderr, "swf_object_output: compress Z_MEM_ERROR: can't malloc\n");
            } else if (result == Z_BUF_ERROR) {
                fprintf(stderr, "swf_object_output: compress Z_BUF_ERROR: not enough buff size\n");
            } else {
                fprintf(stderr, "swf_object_output: compress failed by unknown reason\n");
            }
            bitstream_close(bs);
            return NULL; // FAILURE
        }
        bitstream_putstring(bs, new_buff, compsize);
        free(new_buff);
    } else {
        fprintf(stderr, "swf_object_output: unknown magic %s\n", swf->header.magic);
        bitstream_close(bs);
        return NULL; // FAILURE
    }
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

/* --- */

void
swf_object_print(swf_object_t *swf) {
    int i;
    swf_tag_t *tag;
    swf_header_print(&swf->header);
    swf_header_movie_print(&swf->header_movie);
    tag = swf->tag;
    for (i=0 ; tag ; i++) {
        printf("[%d] ", i);
        swf_tag_print(tag, swf, 0);
        if (tag->tag == 0) { // END Tag
            break;
        }
        tag = tag->next;
    }
}

extern unsigned char *
swf_object_get_tagdata(swf_object_t *swf, int tag_seqno,
                       unsigned long *length) {
    int i;
    swf_tag_t *tag;
    tag = swf->tag;
    for (i=0 ; (i < tag_seqno) &&  tag ; i++) {
        tag = tag->next;
    }
    if (tag) {
        if (tag->data) {
            *length = tag->length;
            return tag->data;
        }
        if (tag->detail == NULL) {
            bitstream_t *bs;
            unsigned char *data;
            bs = bitstream_open();
            swf_tag_build(bs, tag, swf);
            data = bitstream_steal(bs, length);
            bitstream_close(bs);
            return data;
        }
        if (tag->detail) {
            *length = tag->length;
            return tag->data;
        }
    }
    return NULL;
}

int
swf_object_adjust_shapebitmap(swf_object_t *swf, unsigned mode) {
    if (swf == NULL) {
        return 1;
    }
    swf->adjust_shape_bitmap_mode = mode;
    return 0;
}

int
swf_object_replace_tagdata(swf_object_t *swf, int tag_seqno,
                           unsigned char *data, unsigned long *length) {
    fprintf(stderr, "swf_object_replace_tagdata: not impremented yet.\n");
    return 1;
}

/* --- */

unsigned char *
swf_object_get_jpegdata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_t *tag, *tag_jpegtables = NULL;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_jpegdata: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (tag->tag == 8) { // JPEGTables
            tag_jpegtables = tag;
            continue;
        }
        // ! DefineBitsJPEG(1),2,3
        if ((tag->tag != 6) && (tag->tag != 21) && (tag->tag != 35)) {
            continue;
        }
        data = swf_tag_get_jpeg_data(tag, length, image_id, tag_jpegtables);
        if (data) {
            break;
        }
    }
    return data;
}

unsigned char *
swf_object_get_alphadata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_alphadata: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (tag->tag != 35) { // ! DefineBitsJPEG3
            continue;
        }
        data = swf_tag_get_alpha_data(tag, length, image_id);
        if (data) {
            break;
        }
    }
    return data;
}


int
swf_object_replace_jpegdata(swf_object_t *swf, int image_id,
                            unsigned char *jpeg_data,
                            unsigned long jpeg_data_len,
                            unsigned char *alpha_data,
                            unsigned long alpha_data_len) {
    int result = 1;
    swf_tag_t *tag;
    int old_width, old_height, new_width, new_height;
    double width_scale = 0, height_scale = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        register int tag_code = tag->tag;
        // DefineBitsJPEG or 2 or 3, BitsLossless or 2
        if (isBitmapTag(tag_code)) {
            if (swf_tag_identity(tag, image_id) == 0) {
                break; // match
            }
        }
    }
    if (tag == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: tag == NULL\n");
        return 1;
    }
    swf_tag_create_input_detail(tag, swf);
    if (swf->adjust_shape_bitmap_mode) {
        swf_tag_jpeg_detail_t *swf_tag_jpeg = (swf_tag_jpeg_detail_t *) tag->detail;
        jpeg_size(swf_tag_jpeg->jpeg_data, swf_tag_jpeg->jpeg_data_len,
                  &old_width, &old_height);
        jpeg_size(jpeg_data, jpeg_data_len, &new_width, &new_height);
    }
    result = swf_tag_replace_jpeg_data(tag, image_id,
                                       jpeg_data, jpeg_data_len,
                                       alpha_data, alpha_data_len);
    if (result) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf_tag_replace_jpeg_data failed\n");
        return result;
    }
    if (swf->adjust_shape_bitmap_mode & SWFED_SHAPE_BITMAP_MATRIX_RESCALE) {
        width_scale  = (double) old_width  / new_width;
        height_scale = (double) old_height / new_height;
        for (; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_identity(tag, image_id) == 0)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;

                swf_tag_apply_shape_matrix_factor(tag, swf_tag_shape->shape_id,
                                                  width_scale, height_scale,
                                                  0, 0, 0, swf);
            }
        }
    }
    if (swf->adjust_shape_bitmap_mode & SWFED_SHAPE_BITMAP_RECT_RESIZE) {
        width_scale  = (double) new_width  / old_width;
        height_scale = (double) new_height / old_height;
        for (; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_identity(tag, image_id) == 0)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                swf_tag_apply_shape_rect_factor(tag, swf_tag_shape->shape_id,
                                                width_scale, height_scale,
                                                0, 0, swf);
            }
        }
    }
    if (swf->adjust_shape_bitmap_mode & SWFED_SHAPE_BITMAP_TYPE_TILLED) {
        for (; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_identity(tag, image_id) == 0)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                result = swf_tag_apply_shape_type_tilled(tag, swf_tag_shape->shape_id, swf);
            }
        }
    }
    return result;
}

#ifdef HAVE_PNG

unsigned char *
swf_object_get_pngdata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_pngdata: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        // DefineBitsLossless(1),2
        if ((tag->tag != 20) && (tag->tag != 36)) {
            continue;
        }
        data = swf_tag_get_png_data(tag, length, image_id);
        if (data) {
            break;
        }
    }
    return data;
}

int
swf_object_replace_pngdata(swf_object_t *swf, int image_id,
                            unsigned char *png_data,
                            unsigned long png_data_len) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_pngdata: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_replace_png_data(tag, image_id,
                                          png_data, png_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}


#endif /* HAVE_PNG */

#ifdef HAVE_GIF

int
swf_object_replace_gifdata(swf_object_t *swf, int image_id,
                            unsigned char *gif_data,
                            unsigned long gif_data_len) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_gifdata: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_replace_gif_data(tag, image_id,
                                          gif_data, gif_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}

#endif /* HAVE_GIF */

unsigned char *
swf_object_get_sounddata(swf_object_t *swf, unsigned long *length, int sound_id) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_sounddata: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        // DefineSound
        if (tag->tag != 14) {
            continue;
        }
        data = swf_tag_get_sound_data(tag, length, sound_id);
        if (data) {
            break;
        }
    }
    return data;
}

int
swf_object_replace_melodata(swf_object_t *swf, int sound_id,
                            unsigned char *melo_data,
                            unsigned long melo_data_len) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_melodata: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_replace_melo_data(tag, sound_id,
                                           melo_data, melo_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}

char *
swf_object_get_editstring(swf_object_t *swf,
                          char *variable_name,
                          int variable_name_len) {
    swf_tag_t *tag;
    char *data = NULL;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_editstring: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        data = swf_tag_get_edit_string(tag, variable_name,
                                       variable_name_len, swf);
        if (data) {
            break;
        }
    }
    return data;
}
int
swf_object_replace_editstring(swf_object_t *swf,
                              char *variable_name,
                              int variable_name_len,
                              char *initial_text,
                              int initial_text_len) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_editstring: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_replace_edit_string(tag, variable_name,
                                             variable_name_len,
                                             initial_text,
                                             initial_text_len,
                                             swf);
        if (! result) {
            break;
        }
    }
    return result;
}

unsigned char *
swf_object_get_actiondata(swf_object_t *swf, unsigned long *length, int tag_seqno) {
    swf_tag_t *tag;
    swf_tag_action_detail_t *swf_tag_action;
    int i = 0;
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (i == tag_seqno) {
            break;
        }
        i++;
    }
    if (tag == NULL) {
        return NULL;
    }
    if ((tag->tag != 12) &&  (tag->tag != 59)) { //  DoAction, DoInitAction
        return NULL;
    }
    swf_tag_create_input_detail(tag, swf);
    swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    *length = swf_tag_action->action_record_len;
    return swf_tag_action->action_record;
}

int
swf_object_apply_shapematrix_factor(swf_object_t *swf, int shape_id,
                                    double scale_x, double scale_y,
                                    double rotate_rad,
                                    signed int trans_x, signed int trans_y) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_apply_shapematrix_factor: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_matrix_factor(tag, shape_id,
                                                   scale_x, scale_y,
                                                   rotate_rad,
                                                   trans_x, trans_y,
                                                   swf);
        if (! result) {
            break;
        }
    }
    return result;
}

int
swf_object_apply_shaperect_factor(swf_object_t *swf, int shape_id,
                                 double scale_x, double scale_y,
                                 signed int trans_x, signed int trans_y) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_apply_shaperect_factor: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_rect_factor(tag, shape_id,
                                                   scale_x, scale_y,
                                                   trans_x, trans_y,
                                                   swf);
        if (! result) {
            break;
        }
    }
    return result;
}

int
swf_object_apply_shapetype_tilled(swf_object_t *swf,int shape_id) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_apply_shaperect_factor: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_type_tilled(tag, shape_id, swf);
        if (! result) {
            break;
        }
    }
    return result;
}
