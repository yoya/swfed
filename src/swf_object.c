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
#include "swf_action.h"
#include "swf_object.h"

swf_object_t *
swf_object_open(void) {
    swf_object_t *swf;
    malloc_debug_start(); /* DEBUG XXX */
    swf = (swf_object_t *) calloc(sizeof(*swf), 1);
    return swf;
}

void
swf_object_close(swf_object_t *swf) {
    swf_tag_t *tag, *next_tag;
    if (! swf) {
        return ;
    }
    for(tag = swf->tag; tag; tag = next_tag) {
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
    bitstream_t *bs = bitstream_open();
    swf_tag_t **tag;
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
    tag = &swf->tag;
    while(1) {
        long pos;
        pos = bitstream_getbytepos(bs);
        if ((pos == -1) || ((long) swf->header.file_length <= pos)) {
            break;
        }
        *tag = swf_tag_create(bs);
        if (tag == NULL) {
            fprintf(stderr, "swf_object_input: swf_tag_create failed\n");
        }
        tag = &((*tag)->next);
    }
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
    for (tag = swf->tag; tag; tag = tag->next) {
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
        unsigned long compsize;
        unsigned char *new_buff, *old_buff_ref;
        bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
        old_buff_ref = bitstream_buffer(bs, SWF_HEADER_SIZE);
        new_buff = malloc(swf->header.file_length - SWF_HEADER_SIZE);
        result = compress(new_buff, &compsize, old_buff_ref,
                          bs->data_len - SWF_HEADER_SIZE);
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
void
swf_object_print(swf_object_t *swf) {
    int i;
    swf_tag_t *tag;
    swf_header_print(&swf->header);
    swf_header_movie_print(&swf->header_movie);
    tag = swf->tag;
    for (i=0; tag; i++) {
        printf("[%d] ", i);
        swf_tag_print(tag, swf);
        if (tag->tag == 0) { // END Tag
            break;
        }
        tag = tag->next;
    }
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
    for (tag = swf->tag; tag; tag = tag->next) {
        if (tag->tag == 8) { // JPEGTables
            tag_jpegtables = tag;
            break;
        }
    }
    for (tag = swf->tag; tag; tag = tag->next) {
        if (tag->tag == 8) {
            tag_jpegtables = tag;
            continue;
        }
        // DefineBitsJPEG(1),2,3
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
    for (tag = swf->tag; tag; tag = tag->next) {
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
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf == NULL\n");
        return 1;
    }
    for (tag = swf->tag; tag; tag = tag->next) {
        result = swf_tag_replace_jpeg_data(tag, image_id,
                                           jpeg_data, jpeg_data_len,
                                           alpha_data, alpha_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}


unsigned char *
swf_object_get_pngdata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_pngdata: swf == NULL\n");
        return NULL;
    }
    for (tag = swf->tag; tag; tag = tag->next) {
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
    for (tag = swf->tag; tag; tag = tag->next) {
        result = swf_tag_replace_png_data(tag, image_id,
                                           png_data, png_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}

unsigned char *
swf_object_get_sounddata(swf_object_t *swf, unsigned long *length, int sound_id) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_sounddata: swf == NULL\n");
        return NULL;
    }
    for (tag = swf->tag; tag; tag = tag->next) {
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
    for (tag = swf->tag; tag; tag = tag->next) {
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
    for (tag = swf->tag; tag; tag = tag->next) {
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
    for (tag = swf->tag; tag; tag = tag->next) {
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
    for(tag = swf->tag; tag; tag = tag->next) {
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
    swf_tag_create_detail(tag, swf);
    swf_tag_action = (swf_tag_action_detail_t *) tag->detail;
    *length = swf_tag_action->action_record_len;
    return swf_tag_action->action_record;
}
