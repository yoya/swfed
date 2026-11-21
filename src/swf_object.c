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
#include "swf_tag_button.h"
#include "swf_tag_place.h"
#include "swf_tag_sprite.h"
#include "swf_action.h"
#include "swf_object.h"
#include "bitmap_util.h"
#include "trans_table.h"
#ifdef HAVE_LZMA
#include <lzma.h>
#endif

static int _swf_object_remove_tag(swf_object_t *swf, swf_tag_t *tag);
static int _swf_object_remove_tag_in_sprite(swf_tag_sprite_detail_t *sprite, swf_tag_t *tag);
static int _swf_object_replace_tag(swf_object_t *swf,
                                   swf_tag_t *old_tag, swf_tag_t *new_tag);
static void _swf_object_tag_close(swf_tag_t *tag_head);
#ifdef HAVE_LZMA
static int _swf_object_lzma_uncompress(swf_header_t *header, bitstream_t *bs);
static int _swf_object_lzma_compress(swf_object_t *swf, bitstream_t *bs);
#endif

swf_object_t *
swf_object_open(void) {
    swf_object_t *swf;
#ifdef MALLOC_DEBUG
    malloc_debug_start(); /* DEBUG XXX */
#endif // MALLOC_DEBUG
#ifdef BITSTREAM_DEBUG /* bitstream debug */
    bitstream_debug_start();
#endif // BITSTREAM_DEBUG

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
#ifdef MALLOC_DEBUG
    malloc_debug_end(); /* DEBUG XXX */
#endif // MALLOC_DEBUG
#ifdef BITSTREAM_DEBUG /* bitstream debug */
    bitstream_debug_end();
#endif // BITSTREAM_DEBUG
    return ;
}

int
swf_object_input(swf_object_t *swf, unsigned char *data,
                 unsigned long data_len) {
    int result;
    bitstream_t *bs;
    swf_tag_t *tag, *prev_tag;
    /* Parse into temporaries first so a failure part-way through leaves the
     * existing header / tag list untouched. The old content is only replaced
     * once parsing succeeds. */
    swf_header_t new_header;
    swf_header_movie_t new_header_movie;
    swf_tag_t *new_tag_head = NULL;
    swf_tag_t *new_tag_tail = NULL;
    memset(&new_header, 0, sizeof(new_header));
    memset(&new_header_movie, 0, sizeof(new_header_movie));

    bs = bitstream_open();
    if (bitstream_input(bs, data, data_len)) {
        bitstream_close(bs);
        return 1;
    }
    result = swf_header_parse(bs, &new_header);
    if (result) {
        bitstream_close(bs);
        return result;
    }
    if (memcmp(new_header.magic, "FWS", 3) == 0) {
        ; // OK
    } else if (memcmp(new_header.magic, "CWS", 3) == 0) {
        int result;
        unsigned char *old_buff_ref, *new_buff;
        unsigned long origsize, compsize;
        if ((new_header.file_length < SWF_HEADER_SIZE) ||
            (bs->data_len < SWF_HEADER_SIZE)) {
            fprintf(stderr, "swf_object_input: invalid length file_length=%lu data_len=%lu\n",
                    new_header.file_length, bs->data_len);
            bitstream_close(bs);
            return 1; // FAILURE
        }
        old_buff_ref = bitstream_buffer(bs, SWF_HEADER_SIZE);
        compsize = bs->data_len - SWF_HEADER_SIZE;
        origsize = new_header.file_length - SWF_HEADER_SIZE;
        /* deflate can't exceed ~1032:1; don't malloc impossible sizes */
        if (origsize / 1032 > compsize) {
            fprintf(stderr, "swf_object_input: file_length=%lu too large for compressed size=%lu\n",
                    new_header.file_length, compsize);
            bitstream_close(bs);
            return 1; // FAILURE
        }
        new_buff = malloc(origsize ? origsize : 1);
        if (new_buff == NULL) {
            fprintf(stderr, "swf_object_input: Can't malloc(%lu)\n", origsize);
            bitstream_close(bs);
            return 1; // FAILURE
        }
        result = uncompress(new_buff, &origsize, old_buff_ref, compsize);
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
    } else if (memcmp(new_header.magic, "ZWS", 3) == 0) {
#ifdef HAVE_LZMA
        if (_swf_object_lzma_uncompress(&new_header, bs)) {
            bitstream_close(bs);
            return 1; // FAILURE
        }
#else
        fprintf(stderr, "swf_object_input: LZMA compressed SWF (ZWS) is not supported (built without liblzma)\n");
        bitstream_close(bs);
        return 1; // FAILURE
#endif
    } else {
        fprintf(stderr, "swf_object_input: unknown magic %s\n", new_header.magic);
        bitstream_close(bs);
        return 1; // FAILURE
    }
    result = swf_header_movie_parse(bs, &new_header_movie);
    if (result) {
        bitstream_close(bs);
        return result;
    }
    prev_tag = NULL;
    while(1) {
        long pos;
        bitstream_align(bs);
        pos = bitstream_getbytepos(bs);
        if ((pos == -1) || ((long) new_header.file_length <= pos)) {
            break; // a missing End tag at the very end is tolerated
        }
        if ((unsigned long) pos >= bitstream_length(bs)) {
            fprintf(stderr, "swf_object_input: data ends at %ld before file_length(%lu)\n",
                    pos, new_header.file_length);
            _swf_object_tag_close(new_tag_head);
            bitstream_close(bs);
            return 1; // truncated
        }
        tag = swf_tag_create(bs);
        if (tag == NULL) {
            fprintf(stderr, "swf_object_input: broken tag at %ld\n", pos);
            _swf_object_tag_close(new_tag_head);
            bitstream_close(bs);
            return 1;
        }
        if (prev_tag == NULL) {
            new_tag_head = tag;
            tag->prev = tag->next = NULL;
        } else {
            prev_tag->next = tag;
            tag->prev = prev_tag;
            tag->next = NULL;
        }
        new_tag_tail = tag;
        if (tag->code == 0) { // END Tag
            break; // SUCCESS
        }
        prev_tag = tag;
    }
    bitstream_close(bs);

    /* Commit: drop the old content and swap the new content in. */
    _swf_object_tag_close(swf->tag_head);
    swf->tag_head = new_tag_head;
    swf->tag_tail = new_tag_tail;
    swf->header = new_header;
    swf->header_movie = new_header_movie;
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
        if (swf_tag_build(bs, tag, swf)) { // don't emit a SWF with a tag missing
            fprintf(stderr, "swf_object_output: swf_tag_build failed (code=%d)\n", tag->code);
            bitstream_close(bs);
            return NULL;
        }
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
        compsize = compressBound(old_size);
        new_buff = malloc(compsize);
        if (new_buff == NULL) {
            fprintf(stderr, "swf_object_output: Can't malloc(%lu)\n", compsize);
            bitstream_close(bs);
            return NULL; // FAILURE
        }
        result = compress2(new_buff, &compsize, old_buff_ref, old_size, swf->compress_level);
        if (result != Z_OK) {
            if (result == Z_MEM_ERROR) {
                fprintf(stderr, "swf_object_output: compress Z_MEM_ERROR: can't malloc\n");
            } else if (result == Z_BUF_ERROR) {
                fprintf(stderr, "swf_object_output: compress Z_BUF_ERROR: not enough buff size\n");
            } else {
                fprintf(stderr, "swf_object_output: compress failed by unknown reason\n");
            }
            free(new_buff);
            bitstream_close(bs);
            return NULL; // FAILURE
        }
        bitstream_putstring(bs, new_buff, compsize);
        free(new_buff);
    } else if (memcmp(swf->header.magic, "ZWS", SWF_MAGIC_SIZE) == 0) {
#ifdef HAVE_LZMA
        if (_swf_object_lzma_compress(swf, bs)) {
            bitstream_close(bs);
            return NULL; // FAILURE
        }
#else
        fprintf(stderr, "swf_object_output: LZMA compressed SWF (ZWS) is not supported (built without liblzma)\n");
        bitstream_close(bs);
        return NULL; // FAILURE
#endif
    } else {
        fprintf(stderr, "swf_object_output: unknown magic %s\n", swf->header.magic);
        bitstream_close(bs);
        return NULL; // FAILURE
    }
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

#ifdef HAVE_LZMA
/*
 * ZWS: magic(3) version(1) file_length(4) compressed_length(4)
 *      lzma_properties(5) lzma_data
 * liblzma's .lzma (lzma_alone) format: properties(5) uncompressed_size(8) data
 */
#define SWF_LZMA_COMPSIZE_SIZE 4
#define SWF_LZMA_PROPS_SIZE 5
#define SWF_LZMA_DATA_OFFSET (SWF_HEADER_SIZE + SWF_LZMA_COMPSIZE_SIZE + SWF_LZMA_PROPS_SIZE)
#define SWF_LZMA_ALONE_HEADER_SIZE (SWF_LZMA_PROPS_SIZE + 8)

static void
_swf_object_lzma_error(char *func, lzma_ret ret) {
    switch (ret) {
      case LZMA_MEM_ERROR:
        fprintf(stderr, "%s: lzma LZMA_MEM_ERROR: can't malloc\n", func);
        break;
      case LZMA_OPTIONS_ERROR:
        fprintf(stderr, "%s: lzma LZMA_OPTIONS_ERROR: unsupported properties\n", func);
        break;
      case LZMA_FORMAT_ERROR:
        fprintf(stderr, "%s: lzma LZMA_FORMAT_ERROR: invalid properties\n", func);
        break;
      case LZMA_DATA_ERROR:
        fprintf(stderr, "%s: lzma LZMA_DATA_ERROR: corrupt data\n", func);
        break;
      case LZMA_BUF_ERROR:
        fprintf(stderr, "%s: lzma LZMA_BUF_ERROR: truncated data\n", func);
        break;
      default:
        fprintf(stderr, "%s: lzma failed (%d)\n", func, (int) ret);
        break;
    }
}

static int
_swf_object_lzma_uncompress(swf_header_t *header, bitstream_t *bs) {
    unsigned char alone_header[SWF_LZMA_ALONE_HEADER_SIZE];
    unsigned char *data_ref, *new_buff, *tmp;
    unsigned long origsize, alloc_len, dict_size;
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;
    int i, header_fed = 0;
    if ((header->file_length < SWF_HEADER_SIZE) ||
        (bs->data_len < SWF_LZMA_DATA_OFFSET)) {
        fprintf(stderr, "swf_object_input: invalid length file_length=%lu data_len=%lu\n",
                header->file_length, bs->data_len);
        return 1;
    }
    data_ref = bitstream_buffer(bs, 0);
    origsize = header->file_length - SWF_HEADER_SIZE;
    memcpy(alone_header, data_ref + SWF_HEADER_SIZE + SWF_LZMA_COMPSIZE_SIZE,
           SWF_LZMA_PROPS_SIZE);
    // a dictionary larger than the whole movie is never used
    dict_size = alone_header[1] | (alone_header[2] << 8) |
        (alone_header[3] << 16) | ((unsigned long) alone_header[4] << 24);
    if (dict_size > origsize) {
        dict_size = (origsize < 4096) ? 4096 : origsize;
        for (i = 0 ; i < 4 ; i++) {
            alone_header[1 + i] = (dict_size >> (8 * i)) & 0xff;
        }
    }
    // the size is left unknown: the data may or may not have the end marker,
    // and liblzma < 5.4 rejects the end marker when the size is known.
    // decoding stops at file_length instead.
    memset(alone_header + SWF_LZMA_PROPS_SIZE, 0xff, 8);
    ret = lzma_alone_decoder(&strm, UINT64_MAX);
    if (ret != LZMA_OK) {
        _swf_object_lzma_error("swf_object_input", ret);
        return 1;
    }
    // file_length is not trusted: grow the buffer as data comes out.
    alloc_len = (origsize < 0x10000) ? origsize : 0x10000;
    new_buff = malloc(alloc_len ? alloc_len : 1);
    if (new_buff == NULL) {
        fprintf(stderr, "swf_object_input: Can't malloc(%lu)\n", alloc_len);
        lzma_end(&strm);
        return 1;
    }
    strm.next_in = alone_header;
    strm.avail_in = SWF_LZMA_ALONE_HEADER_SIZE;
    strm.next_out = new_buff;
    strm.avail_out = alloc_len;
    while (1) {
        if ((strm.avail_in == 0) && (header_fed == 0)) {
            strm.next_in = data_ref + SWF_LZMA_DATA_OFFSET;
            strm.avail_in = bs->data_len - SWF_LZMA_DATA_OFFSET;
            header_fed = 1;
        }
        if (strm.avail_out == 0) {
            unsigned long new_len = alloc_len * 2;
            if (alloc_len >= origsize) {
                break; // got file_length
            }
            if (new_len > origsize) {
                new_len = origsize;
            }
            tmp = realloc(new_buff, new_len);
            if (tmp == NULL) {
                fprintf(stderr, "swf_object_input: Can't realloc(%lu)\n", new_len);
                free(new_buff);
                lzma_end(&strm);
                return 1;
            }
            new_buff = tmp;
            strm.next_out = new_buff + alloc_len;
            strm.avail_out = new_len - alloc_len;
            alloc_len = new_len;
        }
        ret = lzma_code(&strm, header_fed ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            break;
        }
        if (ret != LZMA_OK) {
            _swf_object_lzma_error("swf_object_input", ret);
            free(new_buff);
            lzma_end(&strm);
            return 1;
        }
    }
    if (strm.total_out != origsize) {
        fprintf(stderr, "swf_object_input: lzma uncompressed size=%llu != file_length=%lu - %d\n",
                (unsigned long long) strm.total_out, header->file_length, SWF_HEADER_SIZE);
        free(new_buff);
        lzma_end(&strm);
        return 1;
    }
    lzma_end(&strm);
    bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
    bitstream_putstring(bs, new_buff, origsize);
    free(new_buff);
    bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
    return 0;
}

static int
_swf_object_lzma_compress(swf_object_t *swf, bitstream_t *bs) {
    lzma_options_lzma opt;
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;
    unsigned char *body_ref, *out, *tmp;
    unsigned long body_len, out_len, data_len;
    uint32_t preset;
    preset = (swf->compress_level <= 9) ? swf->compress_level : LZMA_PRESET_DEFAULT;
    if (lzma_lzma_preset(&opt, preset)) {
        fprintf(stderr, "swf_object_output: lzma_lzma_preset(%u) failed\n", preset);
        return 1;
    }
    ret = lzma_alone_encoder(&strm, &opt);
    if (ret != LZMA_OK) {
        _swf_object_lzma_error("swf_object_output", ret);
        return 1;
    }
    body_ref = bitstream_buffer(bs, SWF_HEADER_SIZE);
    body_len = bs->data_len - SWF_HEADER_SIZE;
    out_len = body_len + body_len / 8 + 1024;
    out = malloc(out_len);
    if (out == NULL) {
        fprintf(stderr, "swf_object_output: Can't malloc(%lu)\n", out_len);
        lzma_end(&strm);
        return 1;
    }
    strm.next_in = body_ref;
    strm.avail_in = body_len;
    strm.next_out = out;
    strm.avail_out = out_len;
    while ((ret = lzma_code(&strm, LZMA_FINISH)) == LZMA_OK) {
        if (strm.avail_out == 0) {
            tmp = realloc(out, out_len * 2);
            if (tmp == NULL) {
                fprintf(stderr, "swf_object_output: Can't realloc(%lu)\n", out_len * 2);
                free(out);
                lzma_end(&strm);
                return 1;
            }
            out = tmp;
            strm.next_out = out + out_len;
            strm.avail_out = out_len;
            out_len *= 2;
        }
    }
    if ((ret != LZMA_STREAM_END) ||
        (strm.total_out < SWF_LZMA_ALONE_HEADER_SIZE)) {
        _swf_object_lzma_error("swf_object_output", ret);
        free(out);
        lzma_end(&strm);
        return 1;
    }
    // drop the uncompressed_size(8) of the .lzma header
    data_len = strm.total_out - SWF_LZMA_ALONE_HEADER_SIZE;
    lzma_end(&strm);
    bitstream_setpos(bs, SWF_HEADER_SIZE, 0);
    bitstream_putbytesLE(bs, data_len, SWF_LZMA_COMPSIZE_SIZE);
    bitstream_putstring(bs, out, SWF_LZMA_PROPS_SIZE);
    bitstream_putstring(bs, out + SWF_LZMA_ALONE_HEADER_SIZE, data_len);
    free(out);
    return 0;
}
#endif /* HAVE_LZMA */

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

/* Register the characters that PlaceObject3, ExportAssets / SymbolClass
 * (linkage) and DoInitAction refer to.
 * PlaceObject3 / ExportAssets / SymbolClass have no detail handler, so their
 * raw data is always there. DoInitAction has one: after rebuild() or
 * replaceActionStrings() the raw data is gone and its SpriteID is in the
 * detail. */
static void
_swf_object_reserve_linkage_refcids(swf_tag_t *tag, trans_table_t *table) {
    unsigned char *d = tag->data, *p;
    unsigned long len = tag->length, pos, i, n;
    if ((d == NULL) && (tag->code == 59) && tag->detail) {
        swf_tag_action_detail_t *action = (swf_tag_action_detail_t *) tag->detail;
        if (action->action_sprite > 0) {
            trans_table_set(table, action->action_sprite, TRANS_TABLE_RESERVE_ID);
        }
        return;
    }
    if (d == NULL) {
        return;
    }
    switch (tag->code) {
      case 59: // DoInitAction: SpriteID
        if (len >= 2) {
            trans_table_set(table, GetUShortLE(d), TRANS_TABLE_RESERVE_ID);
        }
        break;
      case 56: // ExportAssets: Count, { Tag, Name }
      case 76: // SymbolClass: NumSymbols, { Tag, Name }
        if (len < 2) {
            break;
        }
        n = GetUShortLE(d);
        pos = 2;
        for (i = 0 ; (i < n) && (pos + 2 <= len) ; i++) {
            int id;
            p = d + pos;
            id = GetUShortLE(p);
            if (id > 0) { // 0 is the main timeline in SymbolClass
                trans_table_set(table, id, TRANS_TABLE_RESERVE_ID);
            }
            for (pos += 2 ; (pos < len) && d[pos] ; pos++);
            pos++; // NUL
        }
        break;
      case 70: // PlaceObject3
        if (len < 4) {
            break;
        }
        pos = 4; // flags(2), Depth
        // ClassName if HasClassName, or HasImage and HasCharacter
        if ((d[1] & 0x08) || ((d[1] & 0x10) && (d[0] & 0x02))) {
            for ( ; (pos < len) && d[pos] ; pos++);
            pos++;
        }
        if ((d[0] & 0x02) && (pos + 2 <= len)) { // HasCharacter
            p = d + pos;
            trans_table_set(table, GetUShortLE(p), TRANS_TABLE_RESERVE_ID);
        }
        break;
    }
}

int
swf_object_purge_contents(swf_object_t *swf) {
    swf_tag_t *tag, *prev_tag;
    trans_table_t *refcid_trans_table;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_purge_contents: swf == NULL\n");
        return 1;
    }
    refcid_trans_table = trans_table_open();
    if (refcid_trans_table == NULL) {
        fprintf(stderr, "swf_object_purge_contents: trans_table_open failed\n");
        return 1;
    }
    // scan from tail
    for (tag = swf->tag_tail; tag ; tag = prev_tag) {
        int cid;
        int refcid;
        prev_tag = tag->prev;
        _swf_object_reserve_linkage_refcids(tag, refcid_trans_table);
        refcid = swf_tag_get_refcid(tag);
        if (refcid > 0) {
            // register ref id in control tag
            trans_table_set(refcid_trans_table, refcid, TRANS_TABLE_RESERVE_ID);
            continue;
        }
        cid = swf_tag_get_cid(tag);
        if (cid <= 0) {
            continue;
        }
        // contents tag routine is here.
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
            } else if (isButtonTag(tag->code)) {
                int *character_id_list, character_id_list_num;
                character_id_list = swf_tag_button_character_get_refcid_list(tag, &character_id_list_num);
                if (character_id_list) {
                    int i;
                    for (i = 0 ; i < character_id_list_num; i++) {
                        trans_table_set(refcid_trans_table, character_id_list[i], TRANS_TABLE_RESERVE_ID);
                    }
                    free(character_id_list);
                }
            } else if (isSpriteTag(tag->code)) {
                swf_tag_t *t;
                swf_tag_sprite_detail_t *tag_sprite;
                tag_sprite = swf_tag_create_input_detail(tag, swf);
                if (tag_sprite == NULL) {
                    fprintf(stderr, "swf_object_purge_contents: tag_sprite == NULL\n");
                } else {
                    for (t = tag_sprite->tag ; t ; t = t->next) {
                        // PlaceObject3 etc. inside the sprite
                        _swf_object_reserve_linkage_refcids(t, refcid_trans_table);
                        if (isButtonTag(tag->code)) {
                            int *character_id_list, character_id_list_num;
                            character_id_list = swf_tag_button_character_get_refcid_list(tag, &character_id_list_num);
                            if (character_id_list) {
                                int i;
                                for (i = 0 ; i < character_id_list_num; i++) {
                                    trans_table_set(refcid_trans_table, character_id_list[i], TRANS_TABLE_RESERVE_ID);
                                }
                                free(character_id_list);
                            }
                        } else {
                            int rid = swf_tag_get_refcid(t);
                            if (rid > 0) {
                                trans_table_set(refcid_trans_table, rid, TRANS_TABLE_RESERVE_ID);
                            }
                        }
                    }
                }
            }
        } else { // Purge!
            if (isShapeTag(tag->code) || isBitmapTag(tag->code)) {
                _swf_object_remove_tag(swf, tag);
            }
        }
    }
    trans_table_close(refcid_trans_table);
    return 0;
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
            _swf_object_replace_tag(swf, old_tag, new_tag);
            swf_tag_destroy(old_tag);
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
        swf_tag_replace_cid(new_tag, cid); // keep cid in SWF
        if (new_tag) {
            // re-join to new tag
            _swf_object_replace_tag(swf, old_tag, new_tag);
            swf_tag_destroy(old_tag);
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
        /* For edited tags (detail only, no raw data) serialize the detail
         * without the tag header. swf_tag_build() writes the tag header too
         * and also caches the result into tag->data; using it here mixes the
         * header into the returned buffer and poisons tag->data / tag->length
         * for the subsequent output(). Use the detail handler's output
         * directly, which returns just the tag body. */
        if ((tag->data == NULL) && tag->detail) {
            swf_tag_info_t *tag_info = get_swf_tag_info(tag->code);
            const swf_tag_detail_handler_t *handler =
                (tag_info && tag_info->detail_handler)? tag_info->detail_handler(): NULL;
            if (handler && handler->output) {
                unsigned long data_len = 0;
                unsigned char *data = handler->output(tag, &data_len, swf);
                if (data) {
                    tag->data = data;
                    tag->length = data_len;
                }
            }
        }
        if (tag->data && (tag->length >= 2)) {
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
            swf_tag_destroy_detail(tag);
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
                if (tag_sprite == NULL) {
                    return 1;
                }
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

int
swf_object_print_tagdata(swf_object_t *swf, unsigned char *data, 
                         unsigned long length) {
    bitstream_t *bs;
    swf_tag_t *tag;
    bs = bitstream_open();
    bitstream_input(bs, data, length);
    tag = swf_tag_create(bs);
    bitstream_close(bs);
    if (tag == NULL) {
        fprintf(stderr, "swf_object_print_tagdata: swf_tag_create failed\n");
        return 1;
    }
    swf_tag_print(tag, swf, 0);
    swf_tag_destroy(tag);
    return 0;
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
            // fallback reading Shape v0.37 and before (shape body without CID)
            if (new_tag) {
                swf_tag_destroy(new_tag);
            }
            // build a fresh tag; old_tag must stay intact in case this fails
            new_tag = calloc(sizeof(*new_tag), 1);
            if (new_tag == NULL) {
                return 1;
            }
            new_tag->code = old_tag->code;
            new_tag->length_longformat = old_tag->length_longformat;
            new_tag->length = length + 2;
            new_tag->data = malloc(length + 2);
            if (new_tag->data == NULL) {
                free(new_tag);
                return 1;
            }
            PutUShortLE(new_tag->data, cid);
            memcpy(new_tag->data + 2, data, length);
        }
        if (swf_tag_create_input_detail(new_tag, swf) == NULL) {
            swf_tag_destroy(new_tag);
            return 1;
        }
        // save cid in SWF
        swf_tag_replace_cid(new_tag, cid);
        _swf_object_replace_tag(swf, old_tag, new_tag);
        swf_tag_destroy(old_tag);
        // information modified so remove raw data.
        free(new_tag->data);
        new_tag->data = NULL;
        return 0;
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

int
swf_object_search_cid_by_bitmap_condition(swf_object_t *swf,
                                          int width, int height,
                                          int red, int green, int blue) {
    swf_tag_t *tag;
    int cid;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_search_cid_by_bitmap_condition: swf == NULL\n");
        return -1; // NG
    }
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        register int tag_code = tag->code;
        if (isBitmapTag(tag_code)) {
            cid = swf_tag_search_cid_by_bitmap_condition(tag, width, height,
                                                   red, green, blue);
            if (cid > 0) {
                return cid;
            }
        }
    }
    return -1; // Not Found
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
    if ((old_width <= 0) || (old_height <= 0) || (new_width <= 0) || (new_height <= 0)) {
        fprintf(stderr, "swf_object_adjust_shapebitmap: unknown bitmap size (%dx%d -> %dx%d), shapes are not adjusted\n",
                old_width, old_height, new_width, new_height);
        return 1;
    }
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
                                                              bitmap_id,
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
                                                            bitmap_id,
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
                            swf_tag_apply_shape_type_tilled(tag, swf_tag_shape->shape_id, bitmap_id, swf);
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
        if (! isBitsJPEGTag(tag->code)) {
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
        if ((tag->code != 35) && (tag->code != 90)) { // ! DefineBitsJPEG3,4
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
                            unsigned long alpha_data_len,
                            int without_converting) {
    int result = 1;
    swf_tag_t *tag;
    int old_width = 0, old_height = 0, new_width = 0, new_height = 0;
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
        bitmap_size(jpeg_data, jpeg_data_len, &new_width, &new_height);
    }
    result = swf_tag_replace_jpeg_data(tag, image_id,
                                       jpeg_data, jpeg_data_len,
                                       alpha_data, alpha_data_len,
                                       without_converting);
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
                            unsigned long png_data_len, int rgb15) {
    int result = 1;
    swf_tag_t *tag;
    int old_width = 0, old_height = 0, new_width = 0, new_height = 0;
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
                                      png_data, png_data_len, rgb15);
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
    int old_width = 0, old_height = 0, new_width = 0, new_height = 0;
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

int
swf_object_convert_bitmapdata_tojpegtag(swf_object_t *swf) {
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_convert_bitmapdata_tojpegtag: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        swf_tag_convert_bitmap_data_tojpegtag(tag);
    }
    return 0;
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
                          char *variable_name, int variable_name_len,
                          int *error) {
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
                                       variable_name_len, error, swf);
        if (data || (*error == 0)) {
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
    int replaced = 0;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_editstring: swf == NULL\n");
        return 1;
    }
    /* Replace every DefineEditText whose variable name matches. Returning on
     * the first hit leaves duplicate fields with the original text. */
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (swf_tag_replace_edit_string(tag, variable_name,
                                        variable_name_len,
                                        initial_text,
                                        initial_text_len,
                                        swf) == 0) {
            replaced++;
        }
    }
    return replaced? 0: 1;
}

unsigned char *
swf_object_get_actiondata(swf_object_t *swf, unsigned long *length, int tag_seqno) {
    swf_tag_t *tag;
    swf_tag_action_detail_t *swf_tag_action;
    bitstream_t *bs;
    unsigned char *data;
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
    bs = bitstream_open();
    swf_action_list_build(bs,swf_tag_action->action_list);
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

int
swf_object_insert_action_setvariables(swf_object_t *swf,
                                      y_keyvalue_t *kv) {
    swf_tag_t *tag, *prev = NULL;
    swf_tag_t *action_tag = NULL, *prev_tag = NULL;
    int ret, done = 0;
    if ((swf == NULL) || (swf->tag_head == NULL)) {
        fprintf(stderr, "swf_object_insert_action_setvariables: swf or swf->tag_head is NULL\n");
        return 1; // NG
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        switch (tag->code) {
        case 1: // ShowFrame
            if (prev_tag == NULL) {
                // no candidate point before ShowFrame so prev It.
                prev_tag = prev;
            }
            done = 1;
            break;
        case 12: // DoAction
            // DoAction found so merge ActionByteCode to it.
            action_tag = tag;
            done = 1;
            break;
        case 69: // FileAttributs
        case  9: // SetBackgroundColor
        case 24: // Protect
            // put image after there tags.
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
    if (action_tag) { // apppend setvariable image to top of DoAction
        ret = swf_tag_put_action_setvariables(action_tag,
                                              kv,
                                              swf);
        if (ret) {
            fprintf(stderr, "swf_object_insert_action_setvariables: swf_tag_put_action_setvariables failed\n");
            return 1; // NG
        }
    } else { // insert new DoAction
        tag = swf_tag_create_action_setvariables(kv);
        if (tag == NULL) {
            fprintf(stderr, "swf_object_insert_action_setvariables: swf_tag_create_action_setvariables failed\n");
            return 1; // NG
        }
        if (prev_tag == NULL) { // to make sure. abnormal situation.
            tag->next = swf->tag_head;
            if (tag->next) {
                tag->next->prev = tag;
            } else {
                swf->tag_tail = tag;
            }
            swf->tag_head = tag;
            tag->prev = NULL;
        } else {
            tag->next = prev_tag->next;
            if (tag->next) {
                tag->next->prev = tag;
            } else {
                swf->tag_tail = tag;
            }
            prev_tag->next = tag;
            tag->prev = prev_tag;
        }
    }
    return 0; // SUCCESS
}

/* action strings in DefineButton / DefineButton2 and in PlaceObject2 clip actions */
static int
_swf_object_replace_other_action_strings(swf_tag_t *tag, y_keyvalue_t *kv,
                                         swf_object_t *swf, int *modified) {
    int m = 0;
    *modified = 0;
    if (isButtonTag(tag->code)) {
        swf_tag_button_detail_t *button = swf_tag_create_input_detail(tag, swf);
        if (button == NULL) {
            return 1;
        }
        if (tag->code == 7) { // DefineButton
            if (button->actions) {
                if (swf_action_list_replace_strings(button->actions, &m, kv)) {
                    return 1;
                }
                *modified |= m;
            }
        } else if (button->condactions) { // DefineButton2
            swf_button_condaction_t *ca;
            for (ca = button->condactions->head ; ca ; ca = ca->next) {
                if (ca->actions) {
                    if (swf_action_list_replace_strings(ca->actions, &m, kv)) {
                        return 1;
                    }
                    *modified |= m;
                }
            }
        }
    } else if (tag->code == 70) { // PlaceObject3: no detail, its raw data is edited in place
        return swf_tag_place3_replace_action_strings(tag, kv, modified);
    } else if (tag->code == 26) { // PlaceObject2
        /* only those with HasClipActions need parsing */
        if (tag->data && ((tag->length < 1) || ! (tag->data[0] & 0x80))) {
            return 0;
        }
        if (swf_tag_create_input_detail(tag, swf) == NULL) {
            return 1;
        }
        if (swf_tag_place_replace_action_strings(tag, kv, modified,
                                                 swf->header.version)) {
            return 1;
        }
    }
    if (*modified && tag->data) { // rebuild from the detail
        free(tag->data);
        tag->data = NULL;
    }
    return 0;
}

int
swf_object_replace_action_strings(swf_object_t *swf, y_keyvalue_t *kv) {
    swf_tag_t *tag;
    int ret = 0;
    int m;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_action_strings: swf == NULL\n");
        return 1; // NG
    }
    if (kv == NULL) {
        fprintf(stderr, "swf_object_replace_action_strings: kv == NULL\n");
        return 1; // NG
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (isButtonTag(tag->code) || (tag->code == 26) || (tag->code == 70)) {
            ret = _swf_object_replace_other_action_strings(tag, kv, swf, &m);
            if (ret) {
                fprintf(stderr, "swf_object_replace_action_strings: button / clip actions failed\n");
                break;
            }
        } else if (isActionTag(tag->code)) {
            ret = swf_tag_replace_action_strings(tag, kv, &m, swf);
            if (ret) {
                fprintf(stderr, "swf_object_replace_action_strings: swf_tag_replace_action_string failed\n");
                break;
            }
            if (m && tag->data) { // action tag modified
                free(tag->data);
                tag->data = NULL;
            }
        } else if (isSpriteTag(tag->code)) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(tag, swf);
            if (tag_sprite == NULL) {
                fprintf(stderr, "swf_object_replace_action_strings: tag_sprite == NULL\n");
            } else {
                int modified = 0;
                swf_tag_t *t;
                for (t = tag_sprite->tag ; t ; t = t->next) {
                    if ((t->code == 26) || (t->code == 70)) { // PlaceObject2 / 3 with onClipEvent
                        ret = _swf_object_replace_other_action_strings(t, kv, swf, &m);
                        if (ret) {
                            break;
                        }
                        if (m) {
                            modified = 1;
                        }
                    } else if (isActionTag(t->code)) {
                        ret = swf_tag_replace_action_strings(t, kv, &m, swf);
                        if (ret) {
                            fprintf(stderr, "swf_object_replace_action_strings: replace_action_string failed\n");
                            break;
                        }
                        if (t->data) {
                            free(t->data);
                            t->data = NULL;
                        }
                        modified = 1; // action tag modified
                    }    
                }
                if (modified) { // sprite tag rebuild
                    if (tag->data) {
                        free(tag->data);
                        tag->data = NULL;
                    }
                }
            }
        }
    }
    return ret;
}

/*
 * where replaceMovieClip() puts each tag of the inserted SWF
 */
#define SWF_MOVIECLIP_TAG_UNSUPPORTED -1
#define SWF_MOVIECLIP_TAG_DROP         0 // movie-level, meaningless in a sprite
#define SWF_MOVIECLIP_TAG_DEFINE       1 // before the sprite
#define SWF_MOVIECLIP_TAG_CONTROL      2 // into the sprite

static int
_swf_object_movieclip_tag_kind(int tag_no) {
    switch (tag_no) {
      case 8:  // JPEGTables (merged into DefineBitsJPEG beforehand)
      case 9:  // SetBackgroundColor
      case 24: // Protect
      case 41: // ProductInfo
      case 58: // EnableDebugger
      case 63: // DebugID
      case 64: // EnableDebugger2
      case 65: // ScriptLimits
      case 69: // FileAttributes
      case 77: // Metadata
      case 86: // DefineSceneAndFrameLabelData
      case 93: // EnableTelemetry
      case 777: // Reflex
        return SWF_MOVIECLIP_TAG_DROP;
      case 2:  // DefineShape
      case 6:  // DefineBitsJPEG
      case 7:  // DefineButton
      case 10: // DefineFont
      case 11: // DefineText
      case 13: // DefineFontInfo
      case 14: // DefineSound
      case 17: // DefineButtonSound
      case 20: // DefineBitsLossless
      case 21: // DefineBitsJPEG2
      case 22: // DefineShape2
      case 23: // DefineButtonCxform
      case 32: // DefineShape3
      case 33: // DefineText2
      case 34: // DefineButton2
      case 35: // DefineBitsJPEG3
      case 36: // DefineBitsLossless2
      case 37: // DefineEditText
      case 39: // DefineSprite
      case 46: // DefineMorphShape
      case 48: // DefineFont2
      case 56: // ExportAssets
      case 59: // DoInitAction (SpriteID refers to a DefineSprite)
      case 60: // DefineVideoStream
      case 62: // DefineFontInfo2
      case 73: // DefineFontAlignZones
      case 74: // CSMTextSettings
      case 75: // DefineFont3
      case 78: // DefineScalingGrid
      case 83: // DefineShape4
      case 84: // DefineMorphShape2
      case 87: // DefineBinaryData
      case 88: // DefineFontName
      case 90: // DefineBitsJPEG4
      case 91: // DefineFont4
        return SWF_MOVIECLIP_TAG_DEFINE;
      case 0:  // End
      case 1:  // ShowFrame
      case 3:  // FreeCharacter
      case 4:  // PlaceObject
      case 5:  // RemoveObject
      case 12: // DoAction
      case 15: // StartSound
      case 18: // SoundStreamHead
      case 19: // SoundStreamBlock
      case 26: // PlaceObject2
      case 28: // RemoveObject2
      case 43: // FrameLabel
      case 45: // SoundStreamHead2
      case 61: // VideoFrame
      case 66: // SetTabIndex
      case 70: // PlaceObject3
      case 89: // StartSound2
        return SWF_MOVIECLIP_TAG_CONTROL;
    }
    // ImportAssets, DoABC, SymbolClass, unknown tags, ...
    return SWF_MOVIECLIP_TAG_UNSUPPORTED;
}

/*
 * tags whose first UI16 is the ID of a character defined by another tag
 */
static int
_swf_object_is_first_refcid_tag(int tag_no) {
    switch (tag_no) {
      case 3:  // FreeCharacter: CharacterId
      case 5:  // RemoveObject: CharacterId, Depth (RemoveObject2 has Depth only)
      case 13: // DefineFontInfo: FontID
      case 15: // StartSound: SoundId
      case 23: // DefineButtonCxform: ButtonId
      case 59: // DoInitAction: SpriteID
      case 61: // VideoFrame: StreamID
      case 62: // DefineFontInfo2: FontID
      case 73: // DefineFontAlignZones: FontID
      case 74: // CSMTextSettings: TextID
      case 78: // DefineScalingGrid: CharacterId
      case 88: // DefineFontName: FontID
        return 1;
    }
    return 0;
}

// definition tags that have their own character ID
static int
_swf_object_movieclip_defines_cid(int tag_no) {
    return (_swf_object_movieclip_tag_kind(tag_no) == SWF_MOVIECLIP_TAG_DEFINE) &&
        (! _swf_object_is_first_refcid_tag(tag_no)) &&
        (tag_no != 17) && (tag_no != 56); // DefineButtonSound, ExportAssets
}

/*
 * byte offsets of the character IDs that a tag refers to in its raw data
 */
typedef struct _swf_refcid_offsets_ {
    unsigned long *offset;
    int num, alloc;
} _swf_refcid_offsets_t;

static int
_swf_refcid_offsets_add(_swf_refcid_offsets_t *list, unsigned long offset,
                        unsigned long length) {
    if (offset + 2 > length) {
        return 1; // broken
    }
    if (list->num >= list->alloc) {
        int alloc = list->alloc ? list->alloc * 2 : 8;
        unsigned long *tmp = realloc(list->offset, sizeof(*tmp) * alloc);
        if (tmp == NULL) {
            return 1;
        }
        list->offset = tmp;
        list->alloc = alloc;
    }
    list->offset[list->num++] = offset;
    return 0;
}

// DefineButton, DefineButton2: CharacterID of each BUTTONRECORD
static int
_swf_button_refcid_offsets(swf_tag_t *tag, _swf_refcid_offsets_t *list) {
    unsigned char *data = tag->data;
    unsigned long length = tag->length, pos;
    bitstream_t *bs;
    swf_matrix_t matrix;
    swf_cxformwithalpha_t cxform;
    int flags, ret = 1;
    bs = bitstream_open();
    bitstream_input(bs, data, length);
    bitstream_getbytesLE(bs, 2); // ButtonId
    if (tag->code == 34) { // DefineButton2
        bitstream_getbyte(bs);       // TrackAsMenu
        bitstream_getbytesLE(bs, 2); // ActionOffset
    }
    while (1) {
        bitstream_align(bs);
        flags = bitstream_getbyte(bs);
        if (flags == 0) { // CharacterEndFlag
            ret = 0;
            break;
        }
        if ((flags < 0) ||
            _swf_refcid_offsets_add(list, bitstream_getbytepos(bs), length)) {
            break;
        }
        bitstream_getbytesLE(bs, 2); // CharacterID
        bitstream_getbytesLE(bs, 2); // PlaceDepth
        if (swf_matrix_parse(bs, &matrix)) {
            break;
        }
        if (tag->code == 34) {
            if (swf_cxformwithalpha_parse(bs, &cxform)) {
                break;
            }
            bitstream_align(bs);
            if (flags & 0x10) { // ButtonHasFilterList
                unsigned long filter_list_len;
                pos = bitstream_getbytepos(bs);
                if (pos > length) {
                    break;
                }
                filter_list_len = swf_filter_list_length(data + pos, length - pos);
                if (filter_list_len == 0) {
                    break;
                }
                bitstream_setpos(bs, pos + filter_list_len, 0);
            }
            if (flags & 0x20) { // ButtonHasBlendMode
                bitstream_getbyte(bs);
            }
        }
    }
    bitstream_close(bs);
    return ret;
}

// DefineText, DefineText2: FontID of each TEXTRECORD
static int
_swf_text_refcid_offsets(swf_tag_t *tag, _swf_refcid_offsets_t *list) {
    unsigned long length = tag->length, pos, bits;
    bitstream_t *bs;
    swf_rect_t rect;
    swf_matrix_t matrix;
    int flags, glyph_bits, advance_bits, glyph_count, ret = 1;
    bs = bitstream_open();
    bitstream_input(bs, tag->data, length);
    bitstream_getbytesLE(bs, 2); // CharacterID
    if (swf_rect_parse(bs, &rect) || swf_matrix_parse(bs, &matrix)) {
        bitstream_close(bs);
        return 1;
    }
    bitstream_align(bs);
    glyph_bits = bitstream_getbyte(bs);
    advance_bits = bitstream_getbyte(bs);
    while ((glyph_bits >= 0) && (advance_bits >= 0)) {
        bitstream_align(bs);
        flags = bitstream_getbyte(bs);
        if (flags == 0) { // EndOfRecordsFlag
            ret = 0;
            break;
        }
        if (flags < 0) {
            break;
        }
        if (flags & 0x08) { // StyleFlagsHasFont
            if (_swf_refcid_offsets_add(list, bitstream_getbytepos(bs), length)) {
                break;
            }
            bitstream_getbytesLE(bs, 2); // FontID
        }
        if (flags & 0x04) { // StyleFlagsHasColor: RGB, RGBA in DefineText2
            bitstream_getbytesLE(bs, (tag->code == 33) ? 4 : 3);
        }
        if (flags & 0x01) { // StyleFlagsHasXOffset
            bitstream_getbytesLE(bs, 2);
        }
        if (flags & 0x02) { // StyleFlagsHasYOffset
            bitstream_getbytesLE(bs, 2);
        }
        if (flags & 0x08) { // TextHeight
            bitstream_getbytesLE(bs, 2);
        }
        glyph_count = bitstream_getbyte(bs);
        if (glyph_count < 0) {
            break;
        }
        bits = (unsigned long) glyph_count * (glyph_bits + advance_bits);
        pos = bitstream_getbytepos(bs) + (bits + 7) / 8;
        if ((pos > length) || bitstream_setpos(bs, pos, 0)) {
            break;
        }
    }
    bitstream_close(bs);
    return ret;
}

// DefineButtonSound: ButtonId, ButtonSoundChar0-3 (each followed by SOUNDINFO)
static int
_swf_button_sound_refcid_offsets(swf_tag_t *tag, _swf_refcid_offsets_t *list) {
    unsigned char *data = tag->data, *p;
    unsigned long length = tag->length, pos;
    int i, flags;
    if (_swf_refcid_offsets_add(list, 0, length)) { // ButtonId
        return 1;
    }
    pos = 2;
    for (i = 0 ; i < 4 ; i++) {
        if (pos + 2 > length) {
            return 1;
        }
        p = data + pos;
        if (GetUShortLE(p) == 0) { // no sound
            pos += 2;
            continue;
        }
        _swf_refcid_offsets_add(list, pos, length);
        pos += 2;
        if (pos >= length) {
            return 1;
        }
        flags = data[pos++]; // SOUNDINFO
        if (flags & 0x01) { // HasInPoint
            pos += 4;
        }
        if (flags & 0x02) { // HasOutPoint
            pos += 4;
        }
        if (flags & 0x04) { // HasLoops
            pos += 2;
        }
        if (flags & 0x08) { // HasEnvelope
            if (pos >= length) {
                return 1;
            }
            pos += 1 + 8 * (unsigned long) data[pos];
        }
        if (pos > length) {
            return 1;
        }
    }
    return 0;
}

// ExportAssets: Count, { Tag, Name }
static int
_swf_export_refcid_offsets(swf_tag_t *tag, _swf_refcid_offsets_t *list) {
    unsigned char *data = tag->data;
    unsigned long length = tag->length, pos, i, count;
    if (length < 2) {
        return 1;
    }
    count = GetUShortLE(data);
    pos = 2;
    for (i = 0 ; i < count ; i++) {
        if (_swf_refcid_offsets_add(list, pos, length)) {
            return 1;
        }
        for (pos += 2 ; (pos < length) && data[pos] ; pos++);
        if (pos >= length) {
            return 1; // no NUL
        }
        pos++;
    }
    return 0;
}

static int
_swf_object_refcid_offsets(swf_tag_t *tag, _swf_refcid_offsets_t *list) {
    int tag_no = tag->code;
    if ((! _swf_object_is_first_refcid_tag(tag_no)) &&
        (! isButtonTag(tag_no)) && (tag_no != 11) && (tag_no != 33) &&
        (tag_no != 17) && (tag_no != 56)) {
        return 0; // no reference in the raw data
    }
    if (tag->data == NULL) {
        return 1;
    }
    switch (tag_no) {
      case 7:  // DefineButton
      case 34: // DefineButton2
        return _swf_button_refcid_offsets(tag, list);
      case 11: // DefineText
      case 33: // DefineText2
        return _swf_text_refcid_offsets(tag, list);
      case 17: // DefineButtonSound
        return _swf_button_sound_refcid_offsets(tag, list);
      case 56: // ExportAssets
        return _swf_export_refcid_offsets(tag, list);
    }
    return _swf_refcid_offsets_add(list, 0, tag->length); // first UI16
}

// new ID of a character that the inserted SWF defines, 0 if it doesn't define
static int
_swf_object_movieclip_trans_cid(trans_table_t *cid_trans_table, int cid) {
    int to_cid = trans_table_get(cid_trans_table, cid);
    return (to_cid > 0) ? to_cid : 0;
}

/*
 * follow the change of the character IDs that a tag of the inserted SWF
 * refers to. with dry_run, only check that the inserted SWF defines them.
 * returns 1 if the tag refers to a character the inserted SWF doesn't
 * define, or if it is broken.
 */
static int
_swf_object_movieclip_replace_refcids(swf_tag_t *tag,
                                      trans_table_t *cid_trans_table,
                                      int dry_run) {
    int tag_no = tag->code, refcid = 0, to_refcid, i, ret = 0;
    if (isSpriteTag(tag_no)) {
        swf_tag_sprite_detail_t *tag_sprite;
        swf_tag_t *t;
        tag_sprite = swf_tag_create_input_detail(tag, NULL);
        if (tag_sprite == NULL) {
            fprintf(stderr, "swf_object_replace_movieclip: broken DefineSprite in swf_data\n");
            return 1;
        }
        for (t = tag_sprite->tag ; t ; t = t->next) {
            if (_swf_object_movieclip_replace_refcids(t, cid_trans_table, dry_run)) {
                return 1;
            }
        }
        if (! dry_run) { // rebuild from the inner tags
            free(tag->data);
            tag->data = NULL;
        }
        return 0;
    }
    if (isPlaceTag(tag_no) || (tag_no == 70) || (tag_no == 37)) { // PlaceObject, 2, 3, DefineEditText
        refcid = swf_tag_get_refcid(tag);
        if (refcid <= 0) {
            return 0;
        }
        to_refcid = _swf_object_movieclip_trans_cid(cid_trans_table, refcid);
        if (to_refcid == 0) {
            ret = 1;
        } else if ((! dry_run) && (to_refcid != refcid)) {
            swf_tag_replace_refcid(tag, to_refcid);
        }
    } else if (isShapeTag(tag_no)) {
        int *bitmap_id_list, bitmap_id_list_num;
        bitmap_id_list = swf_tag_shape_bitmap_get_refcid_list(tag, &bitmap_id_list_num);
        if (bitmap_id_list == NULL) {
            return 0;
        }
        for (i = 0 ; i < bitmap_id_list_num ; i++) {
            refcid = bitmap_id_list[i];
            to_refcid = _swf_object_movieclip_trans_cid(cid_trans_table, refcid);
            if (to_refcid == 0) {
                ret = 1;
                break;
            }
        }
        free(bitmap_id_list);
        if ((ret == 0) && (! dry_run)) {
            /* all the bitmap fills at once. replacing the IDs one by one
             * chains when the new ID of a bitmap is another one's old ID. */
            swf_tag_shape_bitmap_replace_refcid_by_table(tag, cid_trans_table);
        }
    } else { // in the raw data
        _swf_refcid_offsets_t list = { NULL, 0, 0 };
        if (_swf_object_refcid_offsets(tag, &list)) {
            fprintf(stderr, "swf_object_replace_movieclip: broken tag %d in swf_data\n", tag_no);
            free(list.offset);
            return 1;
        }
        for (i = 0 ; i < list.num ; i++) {
            unsigned char *p = tag->data + list.offset[i];
            refcid = GetUShortLE(p);
            to_refcid = _swf_object_movieclip_trans_cid(cid_trans_table, refcid);
            if (to_refcid == 0) {
                ret = 1;
                break;
            }
            if (! dry_run) {
                PutUShortLE(p, to_refcid);
            }
        }
        free(list.offset);
    }
    if (ret) {
        fprintf(stderr, "swf_object_replace_movieclip: tag %d refers to character %d that swf_data doesn't define\n", tag_no, refcid);
    }
    return ret;
}

static swf_tag_t *
_swf_object_search_jpegtables(swf_object_t *swf) {
    swf_tag_t *tag;
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        if (tag->code == 8) { // JPEGTables
            return tag;
        }
    }
    return NULL;
}

/*
 * DefineBitsJPEG uses the JPEGTables of the SWF it is in.
 * unless the tables are the same as the ones of the SWF it moves into,
 * merge them (=> DefineBitsJPEG2).
 */
static int
_swf_object_merge_jpegtables(swf_object_t *swf, swf_object_t *to_swf) {
    swf_tag_t *tag, *tag_jpegtables, *to_tag_jpegtables;
    tag_jpegtables = _swf_object_search_jpegtables(swf);
    if (tag_jpegtables == NULL) {
        return 0;
    }
    to_tag_jpegtables = _swf_object_search_jpegtables(to_swf);
    if (to_tag_jpegtables && tag_jpegtables->data && to_tag_jpegtables->data &&
        (tag_jpegtables->length == to_tag_jpegtables->length) &&
        (memcmp(tag_jpegtables->data, to_tag_jpegtables->data,
                tag_jpegtables->length) == 0)) {
        return 0;
    }
    for (tag = swf->tag_head ; tag ; tag = tag->next) {
        unsigned char *jpeg_data;
        unsigned long jpeg_data_len;
        int image_id, ret;
        if (tag->code != 6) { // DefineBitsJPEG
            continue;
        }
        image_id = swf_tag_get_cid(tag);
        jpeg_data = swf_tag_get_jpeg_data(tag, &jpeg_data_len, image_id,
                                          tag_jpegtables);
        if (jpeg_data == NULL) {
            fprintf(stderr, "_swf_object_merge_jpegtables: swf_tag_get_jpeg_data failed (image_id=%d)\n", image_id);
            return 1;
        }
        // => DefineBitsJPEG2
        ret = swf_tag_replace_jpeg_data(tag, image_id, jpeg_data, jpeg_data_len,
                                        NULL, 0, 0);
        free(jpeg_data);
        if (ret) {
            fprintf(stderr, "_swf_object_merge_jpegtables: swf_tag_replace_jpeg_data failed (image_id=%d)\n", image_id);
            return 1;
        }
    }
    return 0;
}

// return Sprite tag mapped target path
static swf_tag_t *
swf_object_saerch_sprite_by_target_path2(swf_tag_t *tag_head,
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
    
    // search PlaceObject by instance_name and get ref CID
    for (tag = tag_head ; tag ; tag=tag->next) {
        cid = 0;
        if ((tag->code == 26) || (tag->code == 70)) { // PlaceObject2, 3
            cid = swf_tag_place_get_cid_by_instance_name(tag, instance_name, instance_name_len, swf);
        }
        if (cid > 0) {
            break; // found
        }
    }
    if (cid <= 0) {
        fprintf(stderr,
                "swf_object_saerch_sprite_by_target_path2: not found place target_path=%s (cid=%d)\n",
                target_path, cid);
        return NULL; // not found instance name;
    }
    
    // search DefineSprite by CID
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        if (isSpriteTag(tag->code)) {
            if (swf_tag_get_cid(tag) ==  cid) {
                sprite_tag = tag;
                break;
            }
        }
    }
    if (next_instance_name) { // nested sprite
        if (sprite_tag) {
            swf_tag_sprite_detail_t *tag_sprite;
            tag_sprite = swf_tag_create_input_detail(sprite_tag, swf);
            if (tag_sprite == NULL) {
                fprintf(stderr, "swf_object_saerch_sprite_by_target_path2: tag_sprite swf_tag_create_input_detail failed\n");
            } else {
                sprite_tag = swf_object_saerch_sprite_by_target_path(tag_sprite->tag, next_instance_name, target_path_len - instance_name_len - 1, swf);
            }
        }
    }
    return sprite_tag;
}
swf_tag_t *
swf_object_saerch_sprite_by_target_path(swf_tag_t *tag_head,
					unsigned char *target_path,
					int target_path_len,
					swf_object_t *swf) {
    swf_tag_t *tag;
    unsigned char* target_path2 = (unsigned char*)malloc(target_path_len+1);
    memcpy(target_path2, target_path, target_path_len+1);

    tag = swf_object_saerch_sprite_by_target_path2(tag_head,
                                             target_path2,
                                             target_path_len,
                                             swf);
    free(target_path2);
    return tag;
}

int
swf_object_replace_movieclip(swf_object_t *swf,
                             unsigned char *instance_name, int instance_name_len,
                             unsigned char *swf_data, int swf_data_len) {
    int cid = 0, sprite_cid = 0, ret = 0;
    swf_tag_t *tag = NULL;
    swf_tag_t *sprite_tag = NULL, *prev_sprite_tag = NULL;
    swf_tag_t *sprite_tag_tail = NULL; // last tag in sprite.
    swf_tag_sprite_detail_t *swf_tag_sprite = NULL;
    swf_object_t *swf4sprite = NULL;
    swf_tag_info_t *tag_info = NULL;
    const swf_tag_detail_handler_t *detail_handler = NULL;
    trans_table_t *cid_trans_table = NULL;
    
    if (swf == NULL) {
        fprintf(stderr, "swf_object_replace_movieclip: swf == NULL\n");
        return 1;
    }
    
    // search symbol(SpriteTag) mapped instance_name(target path)
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
    
    // swf data that MC replace to
    swf4sprite = swf_object_open();
    ret = swf_object_input(swf4sprite, swf_data, swf_data_len);
    if (ret) {
        fprintf(stderr, "swf_object_replace_movieclip: swf_object_input (swf_data_len=%d) failed\n", swf_data_len);
        swf_object_close(swf4sprite);
        return ret;
    }

    /* A DefineSprite can't contain definition tags such as DefineSprite.
     * Reject nested sprites before touching swf: walking them would copy
     * every inner level again, costing memory quadratic to the depth. */
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        swf_tag_sprite_detail_t *s;
        swf_tag_t *t;
        if (! isSpriteTag(tag->code)) {
            continue;
        }
        s = swf_tag_create_input_detail(tag, swf4sprite);
        if (s == NULL) {
            fprintf(stderr, "swf_object_replace_movieclip: broken DefineSprite in swf_data\n");
            swf_object_close(swf4sprite);
            return 1;
        }
        for (t = s->tag ; t ; t = t->next) {
            if (isSpriteTag(t->code)) {
                fprintf(stderr, "swf_object_replace_movieclip: nested DefineSprite in swf_data\n");
                swf_object_close(swf4sprite);
                return 1;
            }
        }
    }

    // check all the tags before touching the movie
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        if (_swf_object_movieclip_tag_kind(tag->code) == SWF_MOVIECLIP_TAG_UNSUPPORTED) {
            tag_info = get_swf_tag_info(tag->code);
            fprintf(stderr, "swf_object_replace_movieclip: can't put tag %d (%s) into a movie clip\n",
                    tag->code, tag_info ? tag_info->name : "unknown");
            swf_object_close(swf4sprite);
            return 1;
        }
    }
    if (_swf_object_merge_jpegtables(swf4sprite, swf)) {
        swf_object_close(swf4sprite);
        return 1;
    }

    // old CID
    cid_trans_table = trans_table_open();
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        int cid;
        cid = swf_tag_get_cid(tag);
        if (cid > 0) {
            trans_table_set(cid_trans_table, cid, TRANS_TABLE_RESERVE_ID);
        }
    }
    // new CID of the characters the inserted SWF defines
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        int to_cid;
        if (! _swf_object_movieclip_defines_cid(tag->code)) {
            continue;
        }
        cid = swf_tag_get_cid(tag);
        if (cid <= 0) {
            continue;
        }
        to_cid = trans_table_get(cid_trans_table, cid);
        if (to_cid == TRANS_TABLE_RESERVE_ID) {
            to_cid = trans_table_get_freeid(cid_trans_table);
            trans_table_set(cid_trans_table, cid, to_cid);
            trans_table_set(cid_trans_table, to_cid, TRANS_TABLE_RESERVE_ID);
        } else if (to_cid == 0) {
            trans_table_set(cid_trans_table, cid, cid);
        }
    }
    // every character referred to must be the one the inserted SWF defines
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        if ((_swf_object_movieclip_tag_kind(tag->code) != SWF_MOVIECLIP_TAG_DROP) &&
            _swf_object_movieclip_replace_refcids(tag, cid_trans_table, 1)) {
            trans_table_close(cid_trans_table);
            swf_object_close(swf4sprite);
            return 1;
        }
    }
    
    // clean up innner SpriteTag
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
    
    // extract tag each that type in SWF
    for (tag = swf4sprite->tag_head ; tag ; tag = tag->next) {
        int tag_no = tag->code;
        switch (_swf_object_movieclip_tag_kind(tag_no)) {
          default: // SWF_MOVIECLIP_TAG_DROP
            break;
          case SWF_MOVIECLIP_TAG_DEFINE:
            // change CID
            if (_swf_object_movieclip_defines_cid(tag_no)) {
                cid = swf_tag_get_cid(tag);
                if (cid > 0) {
                    int to_cid = trans_table_get(cid_trans_table, cid);
                    if ((to_cid > 0) && (cid != to_cid)) {
                        if (swf_tag_replace_cid(tag, to_cid)) {
                            fprintf(stderr, "swf_object_replace_movieclip: swf_tag_replace_cid %d => %d failed\n", cid, to_cid);
                            // no stop
                        }
                    }
                }
            }
            _swf_object_movieclip_replace_refcids(tag, cid_trans_table, 0);
            // extract tag before SpriteTag
            {
                swf_tag_t *inserted = swf_tag_move(tag);
                if (prev_sprite_tag) {
                    prev_sprite_tag->next = inserted;
                    inserted->prev = prev_sprite_tag;
                } else {
                    /* sprite_tag was at the head of swf->tag_head; splice
                     * the inserted tag in front and let it become the new
                     * head. */
                    inserted->prev = NULL;
                    swf->tag_head = inserted;
                }
                prev_sprite_tag = inserted;
                prev_sprite_tag->next = sprite_tag;
                sprite_tag->prev = prev_sprite_tag;
            }
            break;
          case SWF_MOVIECLIP_TAG_CONTROL:
            // insert into Sprite
            // follow Character ID change
            _swf_object_movieclip_replace_refcids(tag, cid_trans_table, 0);
            // inplant tag to Sprite
            {
                // the list in a sprite is doubly linked (removeTag() uses prev)
                swf_tag_t *moved = swf_tag_move(tag);
                moved->prev = sprite_tag_tail;
                moved->next = NULL;
                if (sprite_tag_tail == NULL) {
                    swf_tag_sprite->tag = moved;
                } else {
                    sprite_tag_tail->next = moved;
                }
                sprite_tag_tail = moved;
            }
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
                                    int bitmap_id,
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
                                                   bitmap_id,
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
                                  int bitmap_id, 
                                  double scale_x, double scale_y,
                                  signed int trans_x, signed int trans_y) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_apply_shaperect_factor: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_rect_factor(tag, shape_id, bitmap_id,
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
swf_object_apply_shapetype_tilled(swf_object_t *swf,int shape_id,
                                  int bitmap_id) {
    int result = 1;
    swf_tag_t *tag;
    if (swf == NULL) {
        fprintf(stderr, "swf_object_apply_shaperect_factor: swf == NULL\n");
        return 1;
    }
    for (tag=swf->tag_head ; tag ; tag=tag->next) {
        result = swf_tag_apply_shape_type_tilled(tag, shape_id, bitmap_id, swf);
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
        swf_tag_destroy(tag);
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
        swf_tag_destroy(tag);
    }
    bitstream_close(bs);
    return ret;
}

static int
_swf_object_replace_tag(swf_object_t *swf,
                        swf_tag_t *old_tag, swf_tag_t *new_tag) {
    /* old_tag may be the head (prev == NULL) or the tail (next == NULL).
     * Updating prev->next / next->prev unconditionally dereferences NULL
     * for those edge cases. */
    new_tag->prev = old_tag->prev;
    new_tag->next = old_tag->next;
    if (old_tag->prev) {
        old_tag->prev->next = new_tag;
    } else {
        swf->tag_head = new_tag;
    }
    if (old_tag->next) {
        old_tag->next->prev = new_tag;
    } else {
        swf->tag_tail = new_tag;
    }
    old_tag->prev = NULL;
    old_tag->next = NULL;
    return 0;
}
