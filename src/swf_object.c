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
    swf_tag_list_t *tag_list, *next_tag;
    if (swf == NULL) {
        return ;
    }
    for (tag_list = swf->tag_list; tag_list; tag_list = next_tag) {
        next_tag = tag_list->next;
        if (tag_list->node) {
            swf_tag_destroy(tag_list->node);
        }
        free(tag_list);
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
    swf_tag_list_t *tag_list;
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
    swf->tag_list = tag_list = calloc(sizeof(*tag_list), 1);
    while(1) {
        long pos = bitstream_getbytepos(bs);
        if ((pos == -1) || ((long) swf->header.file_length <= pos)) {
            break;
        }
        tag_list->node = swf_tag_create(bs);
        if (tag_list->node == NULL) {
            fprintf(stderr, "swf_object_input: swf_tag_create failed\n");
        }
        tag_list->next = calloc(sizeof(*tag_list), 1);
        tag_list = tag_list->next;
    }
    bitstream_close(bs);
    return 0;
}

unsigned char *
swf_object_output(swf_object_t *swf, unsigned long *length) {
    int result;
    swf_tag_list_t *tag_list;
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
    for (tag_list = swf->tag_list; tag_list && tag_list->node; tag_list = tag_list->next) {
        swf_tag_build(bs, tag_list->node, swf);
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
    swf_tag_list_t *tag_list;
    swf_header_print(&swf->header);
    swf_header_movie_print(&swf->header_movie);
    tag_list = swf->tag_list;
    for (i=0; tag_list && tag_list->node; i++) {
        printf("[%d] ", i);
        swf_tag_print(tag_list->node, swf);
        if (tag_list->node->tag == 0) { // END Tag
            break;
        }
        tag_list = tag_list->next;
    }
}


/* --- */

unsigned char *
swf_object_get_jpegdata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_list_t *tag_list;
    swf_tag_t *tag_jpegtables = NULL;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_jpegdata: swf == NULL\n");
        return NULL;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node; tag_list = tag_list->next) {
        if (tag_list->node->tag == 8) { // JPEGTables
            tag_jpegtables = tag_list->node;
            break;
        }
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node; tag_list = tag_list->next) {
        if (tag_list->node->tag == 8) {
            tag_jpegtables = tag_list->node;
            continue;
        }
        // DefineBitsJPEG(1),2,3
        if ((tag_list->node->tag != 6) &&
            (tag_list->node->tag != 21) &&
            (tag_list->node->tag != 35)) {
            continue;
        }
        data = swf_tag_get_jpeg_data(tag_list->node, length, image_id, tag_jpegtables);
        if (data) {
            break;
        }
    }
    return data;
}

unsigned char *
swf_object_get_alphadata(swf_object_t *swf, unsigned long *length, int image_id) {
    swf_tag_list_t *tag_list;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_alphadata: swf == NULL\n");
        return NULL;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        if (tag_list->node->tag != 35) { // ! DefineBitsJPEG3
            continue;
        }
        data = swf_tag_get_alpha_data(tag_list->node, length,
                                      image_id);
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
    swf_tag_list_t *tag_list;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_jpegdata: swf == NULL\n");
        return 1;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        result = swf_tag_replace_jpeg_data(tag_list->node, image_id,
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
    swf_tag_list_t *tag_list;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_pngdata: swf == NULL\n");
        return NULL;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        // DefineBitsLossless(1),2
        if ((tag_list->node->tag != 20) &&
            (tag_list->node->tag != 36)) {
            continue;
        }
        data = swf_tag_get_png_data(tag_list->node, length, image_id);
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
    swf_tag_list_t *tag_list;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_pngdata: swf == NULL\n");
        return 1;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        result = swf_tag_replace_png_data(tag_list->node, image_id,
                                           png_data, png_data_len);
        if (! result) {
            break;
        }
    }
    return result;
}

unsigned char *
swf_object_get_sounddata(swf_object_t *swf, unsigned long *length, int sound_id) {
    swf_tag_list_t *tag_list;
    unsigned char *data = NULL;
    *length = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_sounddata: swf == NULL\n");
        return NULL;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        // DefineSound
        if (tag_list->node->tag != 14) {
            continue;
        }
        data = swf_tag_get_sound_data(tag_list->node, length, sound_id);
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
    swf_tag_list_t *tag_list;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_melodata: swf == NULL\n");
        return 1;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        result = swf_tag_replace_melo_data(tag_list->node, sound_id,
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
    swf_tag_list_t *tag_list;
    char *data = NULL;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_editstring: swf == NULL\n");
        return NULL;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        data = swf_tag_get_edit_string(tag_list->node, variable_name,
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
    swf_tag_list_t *tag_list;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_editstring: swf == NULL\n");
        return 1;
    }
    for (tag_list = swf->tag_list; tag_list && tag_list->node;
         tag_list = tag_list->next) {
        result = swf_tag_replace_edit_string(tag_list->node,
                                             variable_name,
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
    swf_tag_list_t *tag_list;
    swf_tag_action_detail_t *swf_tag_action;
    int i = 0;
    for(tag_list = swf->tag_list; tag_list && tag_list->node;
        tag_list = tag_list->next) {
        if (i == tag_seqno) {
            break;
        }
        i++;
    }
    if (tag_list == NULL) {
        return NULL;
    }
    if ((tag_list->node->tag != 12) &&  (tag_list->node->tag != 59)) { //  DoAction, DoInitAction
        return NULL;
    }
    swf_tag_create_detail(tag_list->node, swf);
    swf_tag_action = (swf_tag_action_detail_t *) tag_list->node->detail;
    *length = swf_tag_action->action_record_len;
    return swf_tag_action->action_record;
}
