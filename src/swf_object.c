/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcmp, strchr
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

static int _swf_object_remove_tag(swf_object_t *swf, swf_tag_t *tag);
static int _swf_object_remove_tag_in_sprite(swf_tag_sprite_detail_t *sprite, swf_tag_t *tag);
static int _swf_object_replace_tag(swf_object_t *swf,
                                   swf_tag_t *old_tag, swf_tag_t *new_tag);
static void _swf_object_tag_close(swf_tag_t *tag_head);

swf_object_t *
swf_object_open(void) {
    swf_object_t *swf;
    malloc_debug_start(); /* DEBUG XXX */
    swf = (swf_object_t *) calloc(sizeof(*swf), 1);
    swf->tag_head = NULL;
    swf->tag_tail = NULL;
    //
    swf->shape_adjust_mode = 0;
    swf->compress_level = Z_DEFAULT_COMPRESSION;
    return swf;
}

static void
_swf_object_tag_close(swf_tag_t *tag_head) {
    swf_tag_t *tag, *next_tag;
    for (tag = tag_head ; tag ; tag = next_tag) {
        next_tag = tag->next;
        swf_tag_destroy(tag);
    }
    return ;
}

void
swf_object_close(swf_object_t *swf) {
    if (swf) {
        _swf_object_tag_close(swf->tag_head);
        swf->tag_head = NULL;
        swf->tag_tail= NULL;
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
    swf_tag_t *tag, *prev_tag;
    // 2度呼ばれた時は古い tag を削除
    _swf_object_tag_close(swf->tag_head);
    
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
    swf->tag_head = NULL;
    prev_tag = NULL;
    while(1) {
        long pos;
        pos = bitstream_getbytepos(bs);
        if ((pos == -1) || ((long) swf->header.file_length <= pos)) {
            break;
        }
        tag = swf_tag_create(bs);
        if (tag == NULL) {
            swf_tag_t *next_tag;
            for (tag = swf->tag_head ; tag ; tag = next_tag) {
                next_tag = tag->next;
                swf_tag_destroy(tag);
            }
            bitstream_close(bs);
            return 1; // FAILURE
        }
        if (prev_tag == NULL) {
            swf->tag_head = tag;
            tag->prev = tag->next = NULL;
        } else {
            prev_tag->next = tag;
            tag->prev = prev_tag;
            tag->next = NULL;
        }
        swf->tag_tail = tag;
        if (tag->code == 0) { // END Tag
            break; // SUCCESS
        }
        prev_tag = tag;
    }
    bitstream_close(bs);
    return 0;
}

unsigned char *
swf_object_output(swf_object_t *swf, unsigned long *length) {
    int result;
    swf_tag_t *tag = NULL;
    unsigned char *data = NULL;
    bitstream_t *bs = NULL;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_output: swf == NULL\n");
        return NULL;
    }
    if (length == NULL) {
        fprintf(stderr, "swf_object_output: length == NULL\n");
        return NULL;
    }
    *length = 0;
    bs = bitstream_open();
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
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
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
        compsize = old_size * 1.001 + 12; // 稀に増える事もあるので
        new_buff = malloc(compsize);
        result = compress2(new_buff, &compsize, old_buff_ref, old_size, swf->compress_level);
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
    
    i = 0;
    for (tag = swf->tag_head; tag ; tag = tag->next) {
        printf("[%d] ", i);
        swf_tag_print(tag, swf, 0);
        if (tag->code == 0) { // END Tag
            break;
        }
        i++;
    }
}

/* --- */

int
swf_object_rebuild(swf_object_t *swf) {
    swf_tag_t *tag;
    int ret;
    for (tag = swf->tag_head; tag ; tag = tag->next) {
        ret = swf_tag_rebuild(tag, swf);
	if (ret) {
	    return ret;
	}
    }
    return 0;
}

void
swf_object_purge_contents(swf_object_t *swf) {
    swf_tag_t *tag;
    trans_table_t *refcid_trans_table;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_purge_contents: swf == NULL\n");
        return ;
    }
    refcid_trans_table = trans_table_open();
    if (refcid_trans_table == NULL) {
        fprintf(stderr, "swf_object_purge_contents: trans_table_open failed\n");
        return ;
    }
    // 後ろから走査
    for (tag = swf->tag_tail; tag ; tag = tag->prev) {
        int refcid = swf_tag_get_refcid(tag);
        if (refcid > 0) {
            // 制御系タグに参照IDがある場合は登録
            trans_table_set(refcid_trans_table, refcid, TRANS_TABLE_RESERVE_ID);
            continue;
        }
        int cid = swf_tag_get_cid(tag);
        if (cid <= 0) {
            continue;
        }
        // 以下コンテンツ系のタグ
        if (trans_table_get(refcid_trans_table, cid) == TRANS_TABLE_RESERVE_ID) {
            // no purge
            if (isShapeTag(tag->code)) {
                int *bitmap_id_list, bitmap_id_list_num;
                bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
                if (bitmap_id_list) {
                    int i;
                    for (i = 0 ; i < bitmap_id_list_num; i++) {
                        trans_table_set(refcid_trans_table, bitmap_id_list[i], TRANS_TABLE_RESERVE_ID);
                    }
                    free(bitmap_id_list);
                }
            } else if (isSpriteTag(tag->code)) {
                swf_tag_t *t;
                swf_tag_sprite_detail_t *tag_sprite;
                tag_sprite = swf_tag_create_input_detail(tag, swf);
                if (tag_sprite == NULL) {
                    fprintf(stderr, "swf_object_purge_contents: tag_sprite == NULL\n");
                } else {
                    for (t = tag_sprite->tag ; t ; t = t->next) {
                        int rid = swf_tag_get_refcid(t);
                        if (rid > 0) {
                            trans_table_set(refcid_trans_table, rid, TRANS_TABLE_RESERVE_ID);
                        }
                    }
                }
            }
        } else { // Purge!
            if (isShapeTag(tag->code) || isBitmapTag(tag->code)) {
                // TODO:  tag == head ? tag == tail ? OK?
                swf_tag_t *next_tag = tag->next;
                tag->prev->next = tag->next;
                tag->next->prev = tag->prev;
                swf_tag_destroy(tag);
                tag = next_tag;
            }
        }
    }
    trans_table_close(refcid_trans_table);
}

/* --- */

unsigned char *
swf_object_get_tagdata(swf_object_t *swf, int tag_seqno,
                       unsigned long *length) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    tag = swf_object_search_tag_byseqno(swf, tag_seqno);
    if (tag) {
        bitstream_t *bs = bitstream_open();
        swf_tag_build(bs, tag, swf);
        data = bitstream_steal(bs, length);
        bitstream_close(bs);
    }
    return data;
}

int
swf_object_replace_tagdata(swf_object_t *swf, int tag_seqno,
                           unsigned char *data, unsigned long length) {
    swf_tag_t *old_tag, *new_tag;
    old_tag = swf_object_search_tag_byseqno(swf, tag_seqno);
    if (old_tag) {
        bitstream_t *bs = bitstream_open();
        bitstream_input(bs, data,length);
        new_tag = swf_tag_create(bs);
        bitstream_close(bs);
        if (new_tag) {
            // 新しいタグに繋ぎかえる
            _swf_object_replace_tag(swf, old_tag, new_tag);
            swf_tag_destroy(old_tag); // 古いのは消す
            return 0;
        }
    }
    return 1;
}

unsigned char *
swf_object_get_tagdata_bycid(swf_object_t *swf, int cid,
			     unsigned long *length) {
    swf_tag_t *tag;
    unsigned char *data = NULL;
    tag = swf_object_search_tag_bycid(swf, cid);
    if (tag) {
        bitstream_t *bs = bitstream_open();
        swf_tag_build(bs, tag, swf);
        data = bitstream_steal(bs, length);
        bitstream_close(bs);
    }
    return data;
}

int
swf_object_replace_tagdata_bycid(swf_object_t *swf, int cid,
			      unsigned char *data, unsigned long length) {
    swf_tag_t *old_tag, *new_tag;
    old_tag = swf_object_search_tag_bycid(swf, cid);
    if (old_tag) {
        bitstream_t *bs = bitstream_open();
        bitstream_input(bs, data,length);
        new_tag = swf_tag_create(bs);
        bitstream_close(bs);
        swf_tag_replace_cid(new_tag, cid); // SWF 中の cid は維持する
        if (new_tag) {
            // 新しいタグに繋ぎかえる
            _swf_object_replace_tag(swf, old_tag, new_tag);
            swf_tag_destroy(old_tag); // 前のは消す
            return 0;
        }
    }
    return 1;
}

unsigned char *
swf_object_get_tagcontents_bycid(swf_object_t *swf, int cid,
                                  unsigned long *length) {
    swf_tag_t *tag;
    tag = swf_object_search_tag_bycid(swf, cid);
    if (tag) {
        // 編集されている場合は detail を data に戻す
        if ((tag->data == NULL) && tag->detail) {
            bitstream_t *bs;
            bs = bitstream_open();
            swf_tag_build(bs, tag, swf);
            tag->data = bitstream_steal(bs, &(tag->length));
            bitstream_close(bs);
        }
        if (tag->data) {
            *length = tag->length - 2;
            return tag->data + 2; // success
        }
    }
    *length = 0;
    return NULL; // failed
}

int
swf_object_replace_tagcontents_bycid(swf_object_t *swf, int cid,
                                     unsigned char *data,
                                     unsigned long length) {
    swf_tag_t *tag;
    tag = swf_object_search_tag_bycid(swf, cid);
    if (tag) {
        if (tag->detail) {
            swf_tag_destroy(tag);
            tag->detail = NULL;
        }
        if (tag->data) {
            free(tag->data);
            tag->data = NULL;
        }
        tag->length = length + 2;
        tag->data = malloc(length + 2);
        PutUShortLE(tag->data, cid);
        memcpy(tag->data + 2, data, length);
        return 0; // success
    }
    return 1; // failure
}

int
swf_object_remove_tag(swf_object_t *swf, int tag_seqno,
                      int tag_seqno_in_sprite) {
    swf_tag_t *tag;
    int ret = 1;

    tag = swf_object_search_tag_byseqno(swf, tag_seqno);
    if (tag) {
        if (tag_seqno_in_sprite >= 0) {
            if (isSpriteTag(tag->code)) {
                swf_tag_sprite_detail_t   *tag_sprite;
                swf_tag_t *tag_in_sprite;
                tag_sprite = swf_tag_create_input_detail(tag, swf);

                tag_in_sprite = swf_object_search_tag_in_sprite_byseqno(tag_sprite, tag_seqno_in_sprite);
                if (tag_in_sprite) {
                    ret = _swf_object_remove_tag_in_sprite(tag_sprite, tag_in_sprite);
                    if (ret == 0) {
                        free(tag->data);
                        tag->data = NULL;
                    }
                } else {
                    ;
                }
            } else {
                fprintf(stderr, "swf_object_remove_tag: not SpriteTag seqno=%d\n", tag_seqno);
            }
        } else {
            ret = _swf_object_remove_tag(swf, tag);
        }
    }
    return ret;
}

static int
_swf_object_remove_tag(swf_object_t *swf, swf_tag_t *tag) {
    if (tag->prev) {
        if (tag->next) { // prev:O next:O
            tag->prev->next = tag->next;
            tag->next->prev = tag->prev;
        } else {         // prev:O next:X
            tag->prev->next = NULL;
            swf->tag_tail = tag->prev;
        }
    } else {
        if (tag->next) { // prev:X next:O
            tag->next->prev = NULL;
            swf->tag_head = tag->next;
        } else {         // prev:X next:X
            swf->tag_head = NULL;
            swf->tag_tail = NULL;
        }
    }
    swf_tag_destroy(tag);
    return 0;
}

static int
_swf_object_remove_tag_in_sprite(swf_tag_sprite_detail_t *sprite_tag, swf_tag_t *tag) {
    if (tag->prev) {
        if (tag->next) { // prev:O next:O
            tag->prev->next = tag->next;
            tag->next->prev = tag->prev;
        } else {         // prev:O next:X
            tag->prev->next = NULL;
            // swf->tag_tail = tag->prev;
        }
    } else {
        if (tag->next) { // prev:X next:O
            sprite_tag->tag = tag->next;
            tag->next->prev = NULL;
            // swf->tag_heat = tag->next;
        } else {         // prev:X next:X
            sprite_tag->tag = NULL;
            // swf->tag_head = NULL;
            // swf->tag_tail = NULL;
        }
    }
    swf_tag_destroy(tag);
    return 0;
}

/* --- */

unsigned char *
swf_object_get_shapedata(swf_object_t *swf, int cid, unsigned long *length) {
    swf_tag_t *tag;
    unsigned char *data = NULL;

    tag = swf_object_search_tag_bycid(swf, cid);
    if (tag) {
        bitstream_t *bs;
        if (! isShapeTag(tag->code)) {
            fprintf(stderr, "swf_object_get_shapedata: not isShapeTag(%d)\n", tag->code);
            *length = 0;
            return NULL;
        }
        bs = bitstream_open();
        swf_tag_build(bs, tag, swf);
        data = bitstream_steal(bs, length);
        bitstream_close(bs);
    }
    if (data == NULL) {
        *length = 0;
    }
    return data;
}

int
swf_object_replace_shapedata(swf_object_t *swf, int cid,
                             unsigned char *data,
                             unsigned long length) {
    swf_tag_t *old_tag, *new_tag;
    old_tag = swf_object_search_tag_bycid(swf, cid);
    if (old_tag) {
        bitstream_t *bs;
        if (! isShapeTag(old_tag->code)) {
            fprintf(stderr, "swf_object_replace_shapedata: ! isShapeTag(%d)", old_tag->code);
            return 1; // failure
        }
        bs = bitstream_open();
        bitstream_input(bs, data, length);
        new_tag = swf_tag_create(bs);
        bitstream_close(bs);
        if ((new_tag == NULL) || (! isShapeTag(new_tag->code))) {
            fprintf(stderr, "swf_object_replace_shapedata: fallback to read old shape data\n");
            // 0.37 以前で作成した shape をなるべく読めるように
            if (new_tag) {
                swf_tag_destroy(new_tag);
            }
            new_tag = swf_tag_move(old_tag);
            swf_tag_destroy_detail(new_tag);
            new_tag->length = length + 2;
            new_tag->data = malloc(length + 2);
            PutUShortLE(new_tag->data, cid);
            memcpy(new_tag->data + 2, data, length);
        }
        if (new_tag) {
            if (swf_tag_create_input_detail(new_tag, swf)) {
                // SWF 中の cid を維持する
                swf_tag_replace_cid(new_tag, cid);
                // 新しいタグに繋ぎかえる
                _swf_object_replace_tag(swf, old_tag, new_tag);
                swf_tag_destroy(old_tag); // 前のは消す
                free(new_tag->data);
                new_tag->data = NULL;
                return 0;
            }
        }
    }
    return 1;
}

/* --- */

swf_tag_t *
swf_object_search_tag_byseqno(swf_object_t *swf, int tag_seqno) {
    int i;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_search_tag_by_seqno: swf == NULL\n");
        return NULL;
    }
    i=0;
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        if (i >= tag_seqno) {
            break;
        }
        i++;
    }
    return tag;
}

swf_tag_t *
swf_object_search_tag_in_sprite_byseqno(swf_tag_sprite_detail_t *sprite, int tag_seqno) {
    int i;
    swf_tag_t *tag;
    if (sprite == NULL) {
        fprintf(stderr, "swf_object_search_tag_by_seqno: sprite == NULL\n");
        return NULL;
    }
    i=0;
    for (tag = sprite->tag ; tag ; tag = tag->next) {
        if (i >= tag_seqno) {
            break;
        }
        i++;
    }
    return tag;
}

swf_tag_t *
swf_object_search_tag_bycid(swf_object_t *swf, int cid) {
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_search_tag_bycid: swf == NULL\n");
        return NULL;
    }
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        if (swf_tag_get_cid(tag) == cid) {
            break; // match
        }
    }
    return tag;
}

swf_tag_t *
swf_object_search_bitmap_tag(swf_object_t *swf, int bitmap_id) {
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_search_bitmap_tag: swf == NULL\n");
        return NULL;
    }
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        register int tag_code = tag->code;
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
    int *bitmap_id_list, bitmap_id_list_num;
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_MATRIX_RESCALE) {
        width_scale  = (double) old_width  / new_width;
        height_scale = (double) old_height / new_height;
        for (tag = swf->tag_head ; tag ; tag=tag->next) {
            register int tag_code = tag->code;
            if (isShapeTag(tag_code)) {
                bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
                if (bitmap_id_list) {
                    int i;
                    for (i = 0 ; i < bitmap_id_list_num ; i++) { 
                        if (bitmap_id_list[i] == bitmap_id) {
                            swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                            swf_tag_apply_shape_matrix_factor(tag, swf_tag_shape->shape_id,
                                                              width_scale, height_scale,
                                                              0, 0, 0, swf);
                            break;
                        }
                    }
                    free(bitmap_id_list);
                }
            }
        }
    }
    
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_RECT_RESIZE) {
        width_scale  = (double) new_width  / old_width;
        height_scale = (double) new_height / old_height;
        for (tag = swf->tag_head ; tag ; tag=tag->next) {
            register int tag_code = tag->code;
            if (isShapeTag(tag_code)) {
                bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
                if (bitmap_id_list) {
                    int i;
                    for (i = 0 ; i < bitmap_id_list_num ; i++) { 
                        if (bitmap_id_list[i] == bitmap_id) {
                            swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                            swf_tag_apply_shape_rect_factor(tag, swf_tag_shape->shape_id,
                                                            width_scale, height_scale,
                                                            0, 0, swf);
                            break;
                        }
                    }
                    free(bitmap_id_list);
                }
            }
        }
    }
    if (swf->shape_adjust_mode & SWFED_SHAPE_BITMAP_TYPE_TILLED) {
        for (tag = swf->tag_head ; tag ; tag=tag->next) {
            register int tag_code = tag->code;
            if (isShapeTag(tag_code)) {
                bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
                if (bitmap_id_list) {
                    int i;
                    for (i = 0 ; i < bitmap_id_list_num ; i++) { 
                        if (bitmap_id_list[i] == bitmap_id) {
                            swf_tag_shape_detail_t *swf_tag_shape = tag->detail;
                            swf_tag_apply_shape_type_tilled(tag, swf_tag_shape->shape_id, swf);
                        }
                    }
                    free(bitmap_id_list);
                }
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (tag->code == 8) { // JPEGTables
            tag_jpegtables = tag;
            continue;
        }
        // ! DefineBitsJPEG(1),2,3
        if ((tag->code != 6) && (tag->code != 21) && (tag->code != 35)) {
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (tag->code != 35) { // ! DefineBitsJPEG3
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
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_pngdata: swf == NULL\n");
        return NULL;
    }
    if (length == NULL) {
        fprintf(stderr, "swf_object_get_pngdata: length == NULL\n");
        return NULL;
    }
    *length = 0;
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        // DefineBitsLossless(1),2
        if ((tag->code != 20) && (tag->code != 36)) {
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
    if (png_data == NULL) {
        fprintf(stderr, "swf_object_replace_pngdata: png_data == NULL\n");
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
    if (gif_data == NULL) {
        fprintf(stderr, "swf_object_replace_gifdata: gif_data == NULL\n");
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
    if (length == NULL) {
        fprintf(stderr, "swf_object_get_sounddata: length == NULL\n");
        return NULL;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        // DefineSound
        if (tag->code != 14) {
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
    if (melo_data == NULL) {
        fprintf(stderr, "swf_object_replace_melodata: melo_data == NULL\n");
        return 1;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
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
    if (variable_name == NULL) {
        fprintf(stderr, "swf_object_get_editstring: variable_name == NULL\n");
        return NULL;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
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
    if (swf == NULL) {
        fprintf(stderr, "swf_object_get_actiondata: swf == NULL\n");
        return NULL;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (i == tag_seqno) {
            break;
        }
        i++;
    }
    if (tag == NULL) {
        return NULL;
    }
    if ((tag->code != 12) &&  (tag->code != 59)) { //  DoAction, DoInitAction
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
    swf_tag_t *action_tag = NULL, *prev_tag = NULL;
    int ret, done = 0;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_insert_action_setvariables: swf == NULL\n");
        return 1; // NG
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        switch (tag->code) {
        case 1: // ShowFrame
            if (prev_tag == NULL) {
                // ShowFrame までに候補が見つからなければ、ShowFrame の直前
                prev_tag = prev;
            }
            done = 1;
            break;
        case 12: // DoAction
            // DoAction が見つかれば、そこに ActionByteCode を混ぜる。
            action_tag = tag;
            done = 1;
            break;
        case 69: // FileAttributs
        case  9: // SetBackgroundColor
        case 24: // Protect
            // これらのタグより後ろにイメージを置く
            prev_tag = tag;
            break;
        default:
            if (prev_tag == NULL) {
                prev_tag = prev;
            }
            break;
        }
        if (done) {
            break;
        }
        prev = tag;
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
        if (prev_tag == NULL) { // 普通ないけど、念の為。
            tag->next = swf->tag_head;
            tag->next->prev = tag;
            swf->tag_head = tag;
            tag->prev = NULL;
        } else {
            tag->next = prev_tag->next;
            tag->next->prev = tag;
            prev_tag->next = tag;
            tag->prev = prev_tag;
        }
    }
    return 0; // SUCCESS
}

/*
 * 参照側の cid 値を入れ替える
 */
static void
trans_table_replace_refcid_recursive(swf_tag_t *tag, trans_table_t *cid_trans_table) {
    for ( ; tag ; tag=tag->next) {
        int tag_no = tag->code;
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

// ターゲットパスに対応する Sprite タグを返す
static swf_tag_t *
swf_object_saerch_sprite_by_target_path(swf_tag_t *tag_head,
					unsigned char *target_path,
					int target_path_len,
					swf_object_t *swf) {
    swf_tag_t *tag, *sprite_tag = NULL;
    unsigned char *instance_name, *next_instance_name;
    int instance_name_len;
    int cid = 0;

    next_instance_name = (unsigned char *) strchr((char *) target_path, '/');
    if (next_instance_name != NULL) {
        next_instance_name[0] = '\0'; // null terminate
        next_instance_name++;
    }

    instance_name = target_path;
    instance_name_len = strlen((char *) instance_name);
    
    // インスタンス名から PlaceObject を探し、参照している CID を取得する
    for (tag = tag_head ; tag ; tag=tag->next) {
        cid = 0;
        if (tag->code == 26) { // PlaceObject2
            cid = swf_tag_place_get_cid_by_instance_name(tag, instance_name, instance_name_len, swf);
        }
        if (cid > 0) {
            break; // found
        }
    }
    if (cid <= 0) {
        fprintf(stderr,
                "swf_object_saerch_sprite_by_target_path: not found place target_path=%s (cid=%d)\n",
                target_path, cid);
        return NULL; // not found instance name;
    }

    // CID で DefineSprite を探す
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (isSpriteTag(tag->code)) {
            if (swf_tag_get_cid(tag) ==  cid) {
                sprite_tag = tag;
                break;
            }
        }
    }
    if (next_instance_name) { // 入れ子になっている場合
       if (sprite_tag) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(sprite_tag, swf);
            if (tag_sprite == NULL) {
                fprintf(stderr, "swf_object_saerch_sprite_by_target_path: tag_sprite swf_tag_create_input_detail failed\n");
            } else {
	      sprite_tag = swf_object_saerch_sprite_by_target_path(tag_sprite->tag, next_instance_name, target_path_len - instance_name_len - 1, swf);
	    }
       }
    }
    return sprite_tag;
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
    trans_table_t *cid_trans_table = NULL;

    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_movieclip: swf == NULL\n");
        return 1;
    }

    // インスタンス名(ターゲットパス)に対応するシンボル(Spriteタグ)を探す
    sprite_tag = swf_object_saerch_sprite_by_target_path(tag=swf->tag_head,
							 instance_name,
							 instance_name_len,
							 swf);

    if (sprite_tag == NULL) {
        fprintf(stderr, "swf_object_replace_movieclip: sprite_tag == NULL\n");
        return 1; // not found instance name;
    }
    prev_sprite_tag = sprite_tag->prev;
    sprite_cid = swf_tag_get_cid(sprite_tag);

    // MC に差し替える SWF データ
    swf4sprite = swf_object_open();
    ret = swf_object_input(swf4sprite, swf_data, swf_data_len);
    if (ret) {
        fprintf(stderr, "swf_object_replace_movieclip: swf_object_input (swf_data_len=%d) failed\n", swf_data_len);
        return ret;
    }

    // 既存の CID
    cid_trans_table = trans_table_open();
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        int cid;
        cid = swf_tag_get_cid(tag);
        if (cid > 0) {
            trans_table_set(cid_trans_table, cid, TRANS_TABLE_RESERVE_ID);
        }
    }
//    trans_table_print(cid_trans_table);
    
    // Sprite タグの中を綺麗にする
    tag_info = get_swf_tag_info(sprite_tag->code);
    detail_handler = tag_info->detail_handler();
    free(sprite_tag->data);
    sprite_tag->data = NULL;
    sprite_tag->length = 0;
    if (sprite_tag->detail) {
        detail_handler->destroy(sprite_tag);
    }
    sprite_tag->detail = detail_handler->create();
    swf_tag_sprite = sprite_tag->detail;
    swf_tag_sprite->sprite_id = sprite_cid;

    // SWF 中のタグを種類に応じて展開する
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        int tag_no = tag->code;
        switch (tag_no) {
            // tag skip
          default: // misc
          case 3: // FreeCharacter
          case 9: // SetBackgroundColor
            // 16 missing
          case 56: // Export
          case 69: // FileAttributes
          case 74: // CSMTextSettings
          case 8: // JPEGTables // XXX
              ;
            break;
            // Character Tag
          case 2: // DefineShape
          case 6: // DefineBitsJPEG
          case 7: // DefineButton
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
                      if (swf_tag_replace_cid(tag, to_cid)) {
                          fprintf(stderr, "swf_object_replace_movieclip: swf_tag_replace_cid %d => %d failed\n", cid, to_cid);
                          // no stop
                      }
                  }
              }
              if (isShapeTag(tag_no)) {
		  int *bitmap_id_list, bitmap_id_list_num;
		  bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
		  if (bitmap_id_list) {
		      int i;
		      for (i = 0 ; i < bitmap_id_list_num; i++) {
		          int bitmap_id = bitmap_id_list[i];
			  int to_bitmap_id = trans_table_get(cid_trans_table, bitmap_id);
			  if ((to_bitmap_id > 0) && (bitmap_id != to_bitmap_id)) {
			      swf_tag_shape_bitmap_replace_refcid_list(tag, bitmap_id, to_bitmap_id);
			  }
		      }
		      free(bitmap_id_list);
		  }
              } else if (isSpriteTag(tag_no)){
                  swf_tag_sprite_detail_t *s;
                  s = swf_tag_create_input_detail(tag, swf);
                  if (s == NULL) {
                      fprintf(stderr, "swf_object_replace_movieclip: s swf_tag_create_input_detail failed\n");
                  }
                  trans_table_replace_refcid_recursive(s->tag, cid_trans_table);
		  free(tag->data);
		  tag->data = NULL;
              }
              // Sprite の前に展開
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
    trans_table_close(cid_trans_table);
    swf_object_close(swf4sprite);
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
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
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_type_tilled(tag, shape_id, swf);
        if (! result) {
            break;
        }
    }
    return result;
}

int
swf_object_is_shape_tagdata(unsigned char *data, int data_len) {
  bitstream_t *bs;
  swf_tag_t *tag;
  int ret = 0; // default FALSE;

  bs = bitstream_open();
  bitstream_input(bs, data, data_len);
  tag = swf_tag_create(bs);
  if (tag) {
      if (isShapeTag(tag->code)) {
          ret = 1; // TRUE
      }
  }
  bitstream_close(bs);
  return ret;
}

int
swf_object_is_bitmap_tagdata(unsigned char *data, int data_len) {
  bitstream_t *bs;
  swf_tag_t *tag;
  int ret = 0; // default FALSE;

  bs = bitstream_open();
  bitstream_input(bs, data, data_len);
  tag = swf_tag_create(bs);
  if (tag) {
      if (isBitmapTag(tag->code)) {
          ret = 1; // TRUE
      }
  }
  bitstream_close(bs);
  return ret;
}

// 新しいタグに繋ぎかえる
static int
_swf_object_replace_tag(swf_object_t *swf, 
                        swf_tag_t *old_tag, swf_tag_t *new_tag) {
    old_tag->prev->next = new_tag;
    old_tag->next->prev = new_tag;
    new_tag->prev = old_tag->prev;
    new_tag->next = old_tag->next;
    if (new_tag->prev == NULL) {
        swf->tag_head = new_tag;
    }
    if (new_tag->next == NULL) {
        swf->tag_tail = new_tag;
    }
    old_tag->prev = NULL;
    old_tag->next = NULL;
    return 0;
}
