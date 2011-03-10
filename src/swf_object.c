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
#include "swf_tag.h"
#include "swf_tag_action.h"
#include "swf_tag_jpeg.h"
#include "swf_tag_lossless.h"
#include "swf_tag_shape.h"
#include "swf_tag_place.h"
#include "swf_tag_sprite.h"
#include "swf_action.h"
#include "swf_object.h"
#include "bitmap_util.h"
#include "trans_table.h"

swf_object_t *
swf_object_open(void) {
    swf_object_t *swf;
    malloc_debug_start(); /* DEBUG XXX */
    swf = (swf_object_t *) calloc(sizeof(*swf), 1);
    //
    swf->shape_adjust_mode = 0;
    return swf;
}

void
swf_object_close(swf_object_t *swf) {
    if (swf) {
        swf_tag_t *tag, *next_tag;
        for (tag=swf->tag ; tag ; tag=next_tag) {
            next_tag = tag->next;
            swf_tag_destroy(tag);
        }
        free(swf);
    }
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

unsigned char *
swf_object_get_tagdata(swf_object_t *swf, int tag_seqno,
                       unsigned long *length) {
    int i;
    swf_tag_t *tag;
    tag = swf->tag;
    for (i=0 ; (i < tag_seqno) &&  tag ; i++) {
        tag = tag->next;
    }
    if (tag) {
        if (tag->detail) {
            bitstream_t *bs;
            if (tag->data) {
                free(tag->data);
                tag->data = NULL;
            }
            bs = bitstream_open();
            swf_tag_build(bs, tag, swf);
            tag->data = bitstream_steal(bs, &(tag->length));
            bitstream_close(bs);
        }
        if (tag->data) {
            *length = tag->length;
            return tag->data;
        }
    }
    return NULL;
}

int
swf_object_replace_tagdata(swf_object_t *swf, int tag_seqno,
                           unsigned char *data, unsigned long length) {
    int i;
    swf_tag_t *tag;
    tag = swf->tag;
    for (i=0 ; (i < tag_seqno) &&  tag ; i++) {
        tag = tag->next;
    }
    if (tag) {
        if (tag->data) {
            free(tag->data);
        }
        if (tag->detail) { // FIXME
            swf_tag_destroy(tag);
        }
        tag->length = length;
        tag->data = malloc(length);
        memcpy(tag->data, data, length);
        return 0;
    }
    return 1;
}

unsigned char *
swf_object_get_tagcontents_bycid(swf_object_t *swf, int cid,
                                  unsigned long *length) {
    swf_tag_t *tag;
    tag = swf->tag;
    while (tag) {
        if (swf_tag_get_cid(tag) == cid) {
            break; // match
        }
        tag = tag->next;
    }
    if (tag) {
        if (tag->data) {
            *length = tag->length - 2;
            return tag->data + 2;
        }
        if (tag->detail) {
            bitstream_t *bs;
            if (tag->data) {
                free(tag->data);
                tag->data = NULL;
            }
            bs = bitstream_open();
            swf_tag_build(bs, tag, swf);
            tag->data = bitstream_steal(bs, &(tag->length));
            bitstream_close(bs);
        }
    }
    *length = 0;
    return NULL;
}

int
swf_object_replace_tagcontents_bycid(swf_object_t *swf, int cid,
                                     unsigned char *data,
                                     unsigned long length) {
    swf_tag_t *tag;
    tag = swf->tag;
    while (tag) {
        if (swf_tag_get_cid(tag) == cid) {
            break; // match
        }
        tag = tag->next;
    }
    if (tag) {
        if (tag->detail) {
            swf_tag_destroy(tag);
        }
        if (tag->data) {
            free(tag->data);
            tag->data = NULL;
        }
        tag->length = length + 2;
        tag->data = malloc(length + 2);
        tag->data[0] = cid & 0xff;
        tag->data[1] = cid >> 8;
        memcpy(tag->data + 2, data, length);
        return 0; // success
    }
    return 1; // failure
}

unsigned char *
swf_object_get_shapedata(swf_object_t *swf, int cid, unsigned long *length) {
    swf_tag_t *tag;
    tag = swf->tag;
    while (tag) {
        if (swf_tag_get_cid(tag) == cid) {
            break; // match
        }
        tag = tag->next;
    }
    if (tag) {
        if (! isShapeTag(tag->tag)) {
            fprintf(stderr, "swf_object_get_shapedata: not isShapeTag(%d)\n", tag->tag);
            return NULL;
        }
        if (tag->detail) {
            bitstream_t *bs;
            if (tag->data) {
                free(tag->data);
                tag->data = NULL;
            }
            bs = bitstream_open();
            swf_tag_build(bs, tag, swf);
            tag->data = bitstream_steal(bs, &(tag->length));
            bitstream_close(bs);
        }
        if (tag->data) {
            *length = tag->length - 2;
            return tag->data + 2;
        }
    }
    *length = 0;
    return NULL;
}

int
swf_object_replace_shapedata(swf_object_t *swf, int cid,
                             unsigned char *data,
                             unsigned long length) {
    swf_tag_t *tag;
    tag = swf->tag;
    while (tag) {
        if (swf_tag_get_cid(tag) ==  cid) {
            break; // match
        }
        tag = tag->next;
    }
    if (tag) {
        if (! isShapeTag(tag->tag)) {
            return 1; // failure
        }
        if (tag->detail) {
            swf_tag_destroy(tag);
        }
        if (tag->data) {
            free(tag->data);
            tag->data = NULL;
        }
        tag->length = length + 2;
        tag->data = malloc(length + 2);
        tag->data[0] = cid & 0xff;
        tag->data[1] = cid >> 8;
        memcpy(tag->data + 2, data, length);
        return 0; // success
    }
    return 1; // failure
}

/* --- */

swf_tag_t *
swf_object_search_bitmap_tag(swf_object_t *swf, int bitmap_id) {
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_search_bitmap_tag: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        register int tag_code = tag->tag;
        if (isBitmapTag(tag_code)) {
            if (swf_tag_get_cid(tag) == bitmap_id) {
                return tag; // match
            }
        }
    }
    return NULL;
}

/* --- */

int
swf_object_set_shape_adjust_mode(swf_object_t *swf, unsigned mode) {
    if (swf == NULL) {
        return 1;
    }
    swf->shape_adjust_mode = mode;
    return 0;
}

int
swf_object_adjust_shapebitmap(swf_object_t *swf, int bitmap_id,
                              int old_width, int old_height,
                              int new_width, int new_height) {
    swf_tag_t *tag = NULL;
    double width_scale = 0, height_scale = 0;
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_MATRIX_RESCALE) {
        width_scale  = (double) old_width  / new_width;
        height_scale = (double) old_height / new_height;
        for (tag = swf->tag ; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_get_refcid(tag) == bitmap_id)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;

                swf_tag_apply_shape_matrix_factor(tag, swf_tag_shape->shape_id,
                                                  width_scale, height_scale,
                                                  0, 0, 0, swf);
            }
        }
    }
    
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_RECT_RESIZE) {
        width_scale  = (double) new_width  / old_width;
        height_scale = (double) new_height / old_height;
        for (tag = swf->tag ; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_get_refcid(tag) == bitmap_id)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                swf_tag_apply_shape_rect_factor(tag, swf_tag_shape->shape_id,
                                                width_scale, height_scale,
                                                0, 0, swf);
            }
        }
    }
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_TYPE_TILLED) {
        for (tag = swf->tag ; tag ; tag=tag->next) {
            register int tag_code = tag->tag;
            if (isShapeTag(tag_code) && (swf_tag_shape_bitmap_get_refcid(tag) == bitmap_id)) {
                swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                swf_tag_apply_shape_type_tilled(tag, swf_tag_shape->shape_id, swf);
            }
        }
    }
    return 0;
}

int
swf_object_get_bitmap_size(swf_object_t *swf, int bitmap_id,
                           int *width, int *height) {
    swf_tag_t *tag;
    int ret;
    tag = swf_object_search_bitmap_tag(swf, bitmap_id);
    if (tag == NULL) {
        return 1;
    }
    ret = swf_tag_get_bitmap_size(tag, width, height);
    return ret;
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
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf == NULL\n");
        return 1;
    }
    tag = swf_object_search_bitmap_tag(swf, image_id);
    if (tag == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: tag == NULL\n");
        return 1;
    }
    if (swf->shape_adjust_mode) {
        swf_tag_get_bitmap_size(tag, &old_width, &old_height);
        jpeg_size(jpeg_data, jpeg_data_len, &new_width, &new_height);
    }
    result = swf_tag_replace_jpeg_data(tag, image_id,
                                       jpeg_data, jpeg_data_len,
                                       alpha_data, alpha_data_len);
    if (result) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf_tag_replace_jpeg_data failed\n");
        return result;
    }
    if (swf->shape_adjust_mode) {
        swf_object_adjust_shapebitmap(swf, image_id,
                                      old_width, old_height,
                                      new_width, new_height);
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
    int old_width, old_height, new_width, new_height;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_pngdata: swf == NULL\n");
        return 1;
    }
    tag = swf_object_search_bitmap_tag(swf, image_id);
    if (tag == NULL) {
        fprintf(stderr, "swf_object_replace_pngdata: tag == NULL\n");
        return 1;
    }
    if (swf->shape_adjust_mode) {
        swf_tag_get_bitmap_size(tag, &old_width, &old_height);
        png_size(png_data, png_data_len, &new_width, &new_height);
    }
    result = swf_tag_replace_png_data(tag, image_id,
                                      png_data, png_data_len);
    if (result) {
        fprintf(stderr, "swf_object_replace_pngdata: swf_tag_replace_png_data failed\n");
        return result;
    }
    if (swf->shape_adjust_mode) {
        swf_object_adjust_shapebitmap(swf, image_id,
                                      old_width, old_height,
                                      new_width, new_height);
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
    int old_width, old_height, new_width, new_height;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_gifdata: swf == NULL\n");
        return 1;
    }
    tag = swf_object_search_bitmap_tag(swf, image_id);
    if (tag == NULL) {
        fprintf(stderr, "swf_object_replace_gifdata: tag == NULL\n");
        return 1;
    }
    if (swf->shape_adjust_mode) {
        swf_tag_get_bitmap_size(tag, &old_width, &old_height);
        gif_size(gif_data, gif_data_len, &new_width, &new_height);
    }
    result = swf_tag_replace_gif_data(tag, image_id,
                                      gif_data, gif_data_len);
    if (result) {
        fprintf(stderr, "swf_object_replace_pngdata: swf_tag_replace_png_data failed\n");
        return result;
    }
    if (swf->shape_adjust_mode) {
        swf_object_adjust_shapebitmap(swf, image_id,
                                      old_width, old_height,
                                      new_width, new_height);
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
    swf_tag_action = (swf_tag_action_detail_t *) swf_tag_create_input_detail(tag, swf);
    if (swf_tag_action == NULL) {
        fprintf(stderr, "swf_object_get_actiondata: swf_tag_create_input_detail failed");
        return NULL;
    }
    *length = swf_tag_action->action_record_len;
    return swf_tag_action->action_record;
}

int
swf_object_insert_action_setvariables(swf_object_t *swf,
                                      y_keyvalue_t *kv) {
    swf_tag_t *tag, *prev = NULL;
    swf_tag_t *action_tag = NULL, *prev_tag = NULL, *next_tag = NULL;
    int ret, done = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_insert_action_setvariables: swf == NULL\n");
        return 1; // NG
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        switch (tag->tag) {
        case 1: // ShowFrame
            if (next_tag == NULL) {
                prev_tag = prev;
                next_tag = tag;
            }
            done = 1;
            break;
        case 12: // DoAction
            action_tag = tag;
            done = 1;
            break;
        case 69: // FileAttributs
        case  9: // SetBackgroundColor
        case 24: // Protect
            break;
        default:
            if (next_tag == NULL) {
                prev_tag = prev;
                next_tag = tag;
            }
            break;
        }
        if (done) {
            break;
        }
        prev = tag;
    }
    if ((action_tag == NULL) && (next_tag == NULL)) {
        fprintf(stderr, "swf_object_insert_action_setvariables: action_tag == NULL && next_tag == NULL\n");
        return 1; // NG
    }
    if (action_tag) { // DoAction の頭に変数代入イメージを挿入
        ret = swf_tag_put_action_setvariables(action_tag,
                                              kv,
                                              swf);
        if (ret) {
            fprintf(stderr, "swf_object_insert_action_setvariables: swf_tag_put_action_setvariables failed\n");
            return 1; // NG
        }
    } else { // 新規に DoAction を挿入
        tag = swf_tag_create_action_setvariables(kv);
        if (tag == NULL) {
            fprintf(stderr, "swf_object_insert_action_setvariables: swf_tag_create_action_setvariables failed\n");
            return 1; // NG
        }
        if (prev_tag == NULL) {
            swf->tag = tag;
            tag->next = next_tag;
        } else {
            prev_tag->next = tag;
            tag->next = next_tag;
        }
    }
    return 0; // SUCCESS
}

/*
 * 利用している全 depth 値を取得する。
 */
static void
trans_table_reserve_place_depth_recursive(swf_tag_t *tag, trans_table_t *depth_trans_table) {
    for (; tag ; tag=tag->next) {
        int tag_no = tag->tag;
        if (isPlaceTag(tag_no)) {
            swf_tag_place_detail_t *tag_place;
            tag_place = swf_tag_create_input_detail(tag, NULL);
            if (tag_place == NULL) {
                fprintf(stderr, "trans_table_reserve_place_depth_recursive: tag_place swf_tag_create_input_detail failed\n");
                continue; // skip wrong place tag;
            }
            trans_table_set(depth_trans_table, tag_place->depth, TRANS_TABLE_RESERVE_ID);
        } else if (isSpriteTag(tag_no)) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(tag, NULL);
            if (tag_sprite == NULL) {
                fprintf(stderr, "trans_table_reserve_place_depth_recursive: tag_sprite swf_tag_create_input_detail failed\n");
                continue; // skip wrong sprite tag
            }
            trans_table_reserve_place_depth_recursive(tag_sprite->tag, depth_trans_table);
        }
    }
}

/*
 * depth 値を入れ替える
 */
static void
trans_table_replace_place_depth_recursive(swf_tag_t *tag, trans_table_t *depth_trans_table) {
    for ( ; tag ; tag=tag->next) {
        int tag_no = tag->tag;
        if (isPlaceTag(tag_no)) {
            swf_tag_place_detail_t *tag_place;
            int depth;
            tag_place = swf_tag_create_input_detail(tag, NULL);
            if (tag_place == NULL) {
                fprintf(stderr, "trans_table_replace_place_depth_recursive: tag_place swf_tag_create_input_detail failed");
                continue; // skip wrong place tag
                
            }
            depth = tag_place->depth;
            tag_place = tag->detail;
        } else if (isSpriteTag(tag_no)) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(tag, NULL);
            if (tag_sprite == NULL) {
                fprintf(stderr, "trans_table_replace_place_depth_recursive: tag_sprite swf_tag_create_input_detail failed");
                continue; // skip wrong sprite tag
            }
            trans_table_reserve_place_depth_recursive(tag_sprite->tag, depth_trans_table);
        }
    }
}

/*
 * 参照側の cid 値を入れ替える // 未使用
 */
static void
trans_table_replace_refcid_recursive(swf_tag_t *tag, trans_table_t *cid_trans_table) {
    for ( ; tag ; tag=tag->next) {
        int tag_no = tag->tag;
        if (isSpriteTag(tag_no)) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(tag, NULL);
            if (tag_sprite == NULL) {
                fprintf(stderr, "trans_table_replace_refcid_recursive: tag_sprite swf_tag_create_input_detail failed");
            }
            trans_table_replace_refcid_recursive(tag_sprite->tag, cid_trans_table);
        } else if (isPlaceTag(tag_no)) {
            int refcid = swf_tag_get_refcid(tag);
            if (refcid > 0) {
                int to_refcid = trans_table_get(cid_trans_table, refcid);
                if (refcid != to_refcid) {
                    swf_tag_replace_refcid(tag, to_refcid);
                }
            }
        }
    }
}

int
swf_object_replace_movieclip(swf_object_t *swf,
                             unsigned char *instance_name, int instance_name_len,
                             unsigned char *swf_data, int swf_data_len) {
    int cid = 0, sprite_cid = 0, ret = 0;
    swf_tag_t *tag = NULL;
    swf_tag_t *sprite_tag = NULL, *prev_sprite_tag = NULL;
    swf_tag_t *sprite_tag_tail = NULL; // sprite の中の最後の tag
    swf_tag_sprite_detail_t *swf_tag_sprite = NULL;
    swf_object_t *swf4sprite = NULL;
    swf_tag_info_t *tag_info = NULL;
    swf_tag_detail_handler_t *detail_handler = NULL;
    trans_table_t *cid_trans_table;
    trans_table_t *depth_trans_table;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_movieclip: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (tag->tag == 26) { // PlaceObject2
            cid = swf_tag_place_get_cid_by_instance_name(tag, instance_name, instance_name_len, swf);
            if (cid > 0) {
                break; // found
            }
        }
    }
    if (cid <= 0) {
        fprintf(stderr,
                "swf_object_replace_movieclip: place cid(%d) <= 0\n", cid);
        return 1; // not found instance name;
    }
    for (tag=swf->tag ; tag ; tag=tag->next) {
        if (isSpriteTag(tag->tag)) {
            if (swf_tag_get_cid(tag) ==  cid) {
                sprite_tag = tag;
                sprite_cid = cid;
                break;
            }
        }
        prev_sprite_tag = tag;
    }
    if (sprite_tag == NULL) {
        fprintf(stderr, "swf_object_replace_movieclip: sprite_tag == NULL\n");
        return 1; // not found instance name;
    }

    swf4sprite = swf_object_open();
    ret = swf_object_input(swf4sprite, swf_data, swf_data_len);
    if (ret) {
        fprintf(stderr, "swf_object_replace_movieclip: swf_object_input (swf_data_len=%d) failed\n", swf_data_len);
        return ret;
    }
    // 既存の CID
    cid_trans_table = trans_table_open();
    depth_trans_table = trans_table_open();
    for (tag=swf->tag ; tag ; tag=tag->next) {
        int cid;
        cid = swf_tag_get_cid(tag);
        if (cid > 0) {
            trans_table_set(cid_trans_table, cid, TRANS_TABLE_RESERVE_ID);
        }
    }
    // 既存の DEPTH をチェックする
    trans_table_reserve_place_depth_recursive(swf->tag, depth_trans_table);
    
    // Sprite タグの中を綺麗にする
    tag_info = get_swf_tag_info(sprite_tag->tag);
    detail_handler = tag_info->detail_handler();
    free(sprite_tag->data);
    sprite_tag->data = NULL;
    sprite_tag->length = 0;
    if (sprite_tag->detail) {
        fprintf(stderr, "if (sprite_tag->detail)\n");
        detail_handler->destroy(sprite_tag);
    }
    sprite_tag->detail = detail_handler->create();
    swf_tag_sprite = sprite_tag->detail;
    swf_tag_sprite->sprite_id = sprite_cid;

    // SWF 中のタグを種類に応じて展開する
    for (tag=swf4sprite->tag ; tag ; tag=tag->next) {
        int tag_no = tag->tag;
        switch (tag_no) {
            // tag skip
          default: // misc
          case 3: // FreeCharacter
          case 9: // SetBackgroundColor
            // 16 missing
          case 56: // Export
          case 69: // FileAttributes
          case 74: // CSMTextSettings
              ;
            break;
            // Character Tag
          case 2: // DefineShape
          case 6: // DefineBitsJPEG
          case 7: // DefineButton
          case 8: // JPEGTables
          case 10: // DefineFont
          case 11: // DefineText
          case 13: // DefineFontInfo
          case 14: // DefineSound
          case 17: // DefineButtonSound
          case 18: // SoundStreamHead"
          case 19: // SoundStreamBlock
          case 20: // DefineBitsLossless
          case 21: // DefineBitsJPEG2
          case 22: // DefineShape2
          case 32: // DefineShape3
          case 33: // DefineText2
          case 34: // DefineButton2
          case 35: // DefineBitsJPEG3
          case 36: // DefineBitsLossless2
          case 37: // DefineEditText
          case 39: // DefineSprite
          case 46: // DefineMorphShape
          case 48: // DefineFont2
          case 73: // DefineFontAlignZones
          case 75: // DefineFont3
          case 83: // DefineShape4
          case 84: // DefineMorphShape2
          case 88: // DefineFontName
              // CID 変更
              cid = swf_tag_get_cid(tag);
              if (cid > 0) {
                  int to_cid;
                  to_cid = trans_table_get(cid_trans_table, cid);
                  if (to_cid == TRANS_TABLE_RESERVE_ID) {
                      to_cid = trans_table_get_freeid(cid_trans_table);
                      trans_table_set(cid_trans_table, cid, to_cid);
                      trans_table_set(cid_trans_table, to_cid, TRANS_TABLE_RESERVE_ID);
                  } else if (to_cid == 0) {
                      trans_table_set(cid_trans_table, cid, cid);
                      to_cid = cid;
                  }
                  if (cid != to_cid) {
                      swf_tag_replace_cid(tag, to_cid);
                  }
              }
              if (isShapeTag(tag_no)) {
                  int  bitmap_id = swf_tag_shape_bitmap_get_refcid(tag);
                  if (bitmap_id > 0) {
                      int to_bitmap_id = trans_table_get(cid_trans_table, bitmap_id);
                      swf_tag_shape_bitmap_replace_refcid(tag, to_bitmap_id);
                  }
              } else if (isSpriteTag(tag_no)) {
                  swf_tag_sprite_detail_t *s;
                  s = swf_tag_create_input_detail(tag, swf);
                  if (s == NULL) {
                      fprintf(stderr, "swf_object_replace_movieclip: s swf_tag_create_input_detail failed");
                  }
                  // 未使用
                  trans_table_replace_refcid_recursive(s->tag, cid_trans_table);
              }
              // TODO depth が被らないように。
              ;
              // Sprite の前に CID が被らないように展開
              prev_sprite_tag->next = swf_tag_move(tag);
              prev_sprite_tag = prev_sprite_tag->next;
              prev_sprite_tag->next = sprite_tag;
            break;
            // Control Tag
          case 0: // End
          case 1: // ShowFrame
          case 4: // PlaceObject
          case 5: // RemoveObject
          case 12: // DoAction
          case 15: // StartSound
          case 26: // PlaceObject2
          case 28: // RemoveObject2
          case 43: // FrameLabel
          case 59: // DoInitAction
            // Sprite の中に挿入
            // Character ID の変更に追随
              switch (tag_no) {
                int refcid, to_refcid;
                case 4:  // PlaceObject
                case 26: // PlaceObject2
                  refcid = swf_tag_get_refcid(tag);
                  if (refcid > 0) {
                      to_refcid = trans_table_get(cid_trans_table, refcid);
                      if (refcid != to_refcid) {
                          swf_tag_replace_refcid(tag, to_refcid);
                      }
                  }
                  break;
              }
              // Sprite への tag 埋め込み
              if (sprite_tag_tail == NULL) {
                  swf_tag_sprite->tag = swf_tag_move(tag);
                  sprite_tag_tail = swf_tag_sprite->tag;
              } else {
                  sprite_tag_tail->next = swf_tag_move(tag);
                  sprite_tag_tail = sprite_tag_tail->next;
              }
              sprite_tag_tail->next = NULL;
              if (tag_no == 1) { // ShowFrame
                  swf_tag_sprite->frame_count  += 1;
              }
            break;
        }
    }
    trans_table_replace_place_depth_recursive(swf_tag_sprite->tag, depth_trans_table);
    swf_object_close(swf4sprite);
    trans_table_close(cid_trans_table);
    trans_table_close(depth_trans_table);
    return 0;
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
