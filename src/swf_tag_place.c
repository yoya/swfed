/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_tag_place.h"
#include "swf_object.h"

swf_tag_detail_handler_t place_detail_handler;

swf_tag_detail_handler_t *
swf_tag_place_detail_handler(void) {
    place_detail_handler.create   = swf_tag_place_create_detail;
    place_detail_handler.input    = swf_tag_place_input_detail;
    place_detail_handler.get_cid  = NULL;
    place_detail_handler.replace_cid = NULL;
    place_detail_handler.output   = swf_tag_place_output_detail;
    place_detail_handler.print    = swf_tag_place_print_detail;
    place_detail_handler.destroy  = swf_tag_place_destroy_detail;
    return &place_detail_handler;
}

void *
swf_tag_place_create_detail(void) {
    swf_tag_place_detail_t *swf_tag_place;
    swf_tag_place = calloc(sizeof(*swf_tag_place), 1);
    if (swf_tag_place == NULL) {
        fprintf(stderr, "ERROR: swf_tag_place_create_detail: can't calloc\n");
        return NULL;
    }
    return swf_tag_place;
}

int
swf_tag_place_input_detail(swf_tag_t *tag, struct swf_object_ *swf) {
    swf_tag_place_detail_t *swf_tag_place = tag->detail;
    unsigned char *data  = tag->data;
    unsigned long length = tag->length;
    bitstream_t *bs;
    int ret;
    (void) swf;
    if (swf_tag_place == NULL) {
        fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place == NULL\n");
        return 1;
    }
    swf_tag_place->character_id = 0; // undefined
    bs = bitstream_open();
    bitstream_input(bs, data, length);

    if (tag->code == 4) { // PlaceObject
        swf_tag_place->character_id = bitstream_getbytesLE(bs, 2);
        swf_tag_place->depth = bitstream_getbytesLE(bs, 2);
        ret = swf_matrix_parse(bs, &(swf_tag_place->matrix));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->matrix parse failed. character_id=%d\n", swf_tag_place->character_id);
            bitstream_close(bs);
            return ret;
        }
        bitstream_align(bs);
        if (bitstream_getbytepos(bs) < length) { // optional
            ret = swf_cxform_parse(bs, &(swf_tag_place->color_transform));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->color_transform parse failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return ret;
            }
        } else {
            swf_tag_place->color_transform.has_add_terms = 0;
            swf_tag_place->color_transform.has_mult_terms = 0;
            swf_tag_place->color_transform.nbits = 0;
        }
    } else if (tag->code == 26) { // PlaceObject2
        swf_tag_place->flag_has_clip_action = bitstream_getbit(bs);
        swf_tag_place->flag_has_clip_depth = bitstream_getbit(bs);
        swf_tag_place->flag_has_name = bitstream_getbit(bs);
        swf_tag_place->flag_has_ratio = bitstream_getbit(bs);
        swf_tag_place->flag_has_color_transform = bitstream_getbit(bs);
        swf_tag_place->flag_has_matrix = bitstream_getbit(bs);
        swf_tag_place->flag_has_character = bitstream_getbit(bs);
        swf_tag_place->flag_has_move = bitstream_getbit(bs);
        swf_tag_place->depth = bitstream_getbytesLE(bs, 2);
        if (swf_tag_place->flag_has_character) {
            swf_tag_place->character_id = bitstream_getbytesLE(bs, 2);
        } else {
            swf_tag_place->character_id = 0;
        }
        if (swf_tag_place->flag_has_matrix) {
            ret = swf_matrix_parse(bs, &(swf_tag_place->matrix));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->matrix parse failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return ret;
            }
        }
        if (swf_tag_place->flag_has_color_transform) {
            ret = swf_cxformwithalpha_parse(bs, &(swf_tag_place->color_transform_with_alpha));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->color_transform parse failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return ret;
            }
        }
        if (swf_tag_place->flag_has_ratio) {
            swf_tag_place->ratio = bitstream_getbytesLE(bs, 2);
        }
        if (swf_tag_place->flag_has_name) {
            swf_tag_place->name = (char *) bitstream_outputstring(bs);
            if (swf_tag_place->name == NULL) {
                fprintf(stderr, "ERROR: swf_tag_place_input_detail: broken name. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return 1;
            }
        }
        if (swf_tag_place->flag_has_clip_depth) {
            swf_tag_place->clip_depth = bitstream_getbytesLE(bs, 2);   
        }
        // TODO: clip action data for SWF 5
        
    } else {
        bitstream_close(bs);
        return 1; // unknown tag;
    }
    bitstream_close(bs);
    return 0;
}

unsigned char *
swf_tag_place_output_detail(swf_tag_t *tag, unsigned long *length,
                           struct swf_object_ *swf) {
    swf_tag_place_detail_t *swf_tag_place = (swf_tag_place_detail_t *) tag->detail;
    bitstream_t *bs;
    unsigned char *data;
    int ret;
    (void) swf;
    *length = 0;

    bs = bitstream_open();

    if (tag->code == 4) { // PlaceObject
        bitstream_putbytesLE(bs, swf_tag_place->character_id, 2);
        bitstream_putbytesLE(bs, swf_tag_place->depth, 2);
        ret = swf_matrix_build(bs, &(swf_tag_place->matrix));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_output_detail: swf_tag_place->matrix build failed. character_id=%d\n", swf_tag_place->character_id);
            bitstream_close(bs);
            return NULL;
        }
        if (swf_tag_place->color_transform.has_add_terms ||
            swf_tag_place->color_transform.has_mult_terms) { // optional
            ret = swf_cxform_build(bs, &(swf_tag_place->color_transform));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_output_detail: swf_tag_place->color_transform build failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return NULL;
            }
        }
    } else if (tag->code == 26) { // PlaceObject2
        bitstream_putbit(bs, swf_tag_place->flag_has_clip_action);
        bitstream_putbit(bs, swf_tag_place->flag_has_clip_depth);
        bitstream_putbit(bs, swf_tag_place->flag_has_name);
        bitstream_putbit(bs, swf_tag_place->flag_has_ratio);
        bitstream_putbit(bs, swf_tag_place->flag_has_color_transform);
        bitstream_putbit(bs, swf_tag_place->flag_has_matrix);
        bitstream_putbit(bs, swf_tag_place->flag_has_character);
        bitstream_putbit(bs, swf_tag_place->flag_has_move);
        
        bitstream_putbytesLE(bs, swf_tag_place->depth, 2);

        if (swf_tag_place->flag_has_character) {
            bitstream_putbytesLE(bs, swf_tag_place->character_id, 2);
        }
        if (swf_tag_place->flag_has_matrix) {
            ret = swf_matrix_build(bs, &(swf_tag_place->matrix));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_output_detail: swf_tag_place->matrix build failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return NULL;
            }
        }
        if (swf_tag_place->flag_has_color_transform) {
            ret = swf_cxformwithalpha_build(bs, &(swf_tag_place->color_transform_with_alpha));
            if (ret) {
                fprintf(stderr, "ERROR: swf_tag_place_output_detail: swf_tag_place->color_transform build failed. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return NULL;
            }
        }
        if (swf_tag_place->flag_has_ratio) {
            bitstream_putbytesLE(bs, swf_tag_place->ratio, 2);
        }
        if (swf_tag_place->flag_has_name) {
            bitstream_putstring(bs, (unsigned char *) swf_tag_place->name, strlen(swf_tag_place->name) + 1);
        }
        if (swf_tag_place->flag_has_clip_depth) {
            bitstream_putbytesLE(bs, swf_tag_place->clip_depth, 2);
        }
        // TODO: clip action data for SWF 5
    } else {
        bitstream_close(bs);
        return NULL; // unknown tag;
    }
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
}

void
swf_tag_place_print_detail(swf_tag_t *tag,
                           struct swf_object_ *swf, int indent_depth) {
    swf_tag_place_detail_t *swf_tag_place = (swf_tag_place_detail_t *) tag->detail;
    (void) swf;
    print_indent(indent_depth);
    if (tag->code == 4) { // PlaceObject
        printf("character_id=%d  depth=%d\n",
               swf_tag_place->character_id, swf_tag_place->depth);
        swf_matrix_print(&(swf_tag_place->matrix), indent_depth);
        swf_cxform_print(&(swf_tag_place->color_transform), indent_depth);
    } else if (tag->code == 26) { // PlaceObject2
        printf("(clipact,clipdepth,name,ratio,coltrans,mat,cid,move)=(%d,%d,%d,%d,%d,%d,%d,%d)\n",
               swf_tag_place->flag_has_clip_action,
               swf_tag_place->flag_has_clip_depth,
               swf_tag_place->flag_has_name,
               swf_tag_place->flag_has_ratio,
               swf_tag_place->flag_has_color_transform,
               swf_tag_place->flag_has_matrix,
               swf_tag_place->flag_has_character,
               swf_tag_place->flag_has_move);
        if (swf_tag_place->flag_has_character) {
            print_indent(indent_depth);
            printf("character_id=%d\n", swf_tag_place->character_id);
        }
        if (swf_tag_place->flag_has_matrix) {
            swf_matrix_print(&(swf_tag_place->matrix), indent_depth);
        }
        if (swf_tag_place->flag_has_color_transform) {
            swf_cxformwithalpha_print(&(swf_tag_place->color_transform_with_alpha), indent_depth);
        }
        if (swf_tag_place->flag_has_ratio) {
            print_indent(indent_depth);
            printf("ratio=%d\n", swf_tag_place->ratio);
        }
        if (swf_tag_place->flag_has_name) {
            print_indent(indent_depth);
            printf("name=%s\n", swf_tag_place->name);
        }
        if (swf_tag_place->flag_has_clip_depth) {
            print_indent(indent_depth);
            printf("ratio=%d\n", swf_tag_place->clip_depth);
        }
    } else {
        fprintf(stderr, "Illegal tag no(%d)\n", tag->code);
    }
    return ;
}

void
swf_tag_place_destroy_detail(swf_tag_t *tag) {
    swf_tag_place_detail_t *swf_tag_place = (swf_tag_place_detail_t *) tag->detail;
    if (swf_tag_place) {
        if (swf_tag_place->name) {
            free(swf_tag_place->name);
        }
        free(swf_tag_place);
        tag->detail = NULL;
    }
    return ;
}

/*
 * PlaceObject3 has no detail handler (its raw data is always kept):
 * read the fields up to Name.
 * *cid_offset: byte offset of CharacterId, 0 if it has no CharacterId.
 * *name (if name != NULL): malloc'ed instance name, NULL if it has no Name.
 */
int
swf_tag_place3_parse_head(swf_tag_t *tag, unsigned long *cid_offset, char **name) {
    bitstream_t *bs;
    int flags, flags2;
    swf_matrix_t matrix;
    swf_cxformwithalpha_t cxform;
    *cid_offset = 0;
    if (name) {
        *name = NULL;
    }
    if ((tag->code != 70) || (tag->data == NULL) || (tag->length < 4)) {
        return 1;
    }
    bs = bitstream_open();
    bitstream_input(bs, tag->data, tag->length);
    flags = bitstream_getbyte(bs);
    flags2 = bitstream_getbyte(bs);
    bitstream_getbytesLE(bs, 2); // Depth
    // ClassName: HasClassName, or HasImage and HasCharacter
    if ((flags2 & 0x08) || ((flags2 & 0x10) && (flags & 0x02))) {
        char *class_name = (char *) bitstream_outputstring(bs);
        if (class_name == NULL) {
            bitstream_close(bs);
            return 1;
        }
        free(class_name);
    }
    if (flags & 0x02) { // HasCharacter
        *cid_offset = bitstream_getbytepos(bs);
        if (*cid_offset + 2 > tag->length) {
            *cid_offset = 0;
            bitstream_close(bs);
            return 1;
        }
        bitstream_getbytesLE(bs, 2);
    }
    if ((name == NULL) || ((flags & 0x20) == 0)) { // ! HasName
        bitstream_close(bs);
        return 0;
    }
    if ((flags & 0x04) && swf_matrix_parse(bs, &matrix)) { // HasMatrix
        bitstream_close(bs);
        return 1;
    }
    if ((flags & 0x08) && swf_cxformwithalpha_parse(bs, &cxform)) { // HasColorTransform
        bitstream_close(bs);
        return 1;
    }
    if (flags & 0x10) { // HasRatio
        bitstream_getbytesLE(bs, 2);
    }
    *name = (char *) bitstream_outputstring(bs);
    bitstream_close(bs);
    return (*name == NULL) ? 1 : 0;
}

int
swf_tag_place_get_cid_by_instance_name(swf_tag_t *tag, unsigned char *instance_name, int instance_name_len, struct swf_object_ *swf) {
    swf_tag_place_detail_t *swf_tag_place;
    if (tag->code == 70) { // PlaceObject3
        unsigned long cid_offset;
        char *name;
        int cid, match;
        if (swf_tag_place3_parse_head(tag, &cid_offset, &name)) {
            return -1;
        }
        if (name == NULL) {
            return -2; // no name
        }
        match = (strlen(name) == (size_t) instance_name_len) &&
            (strncmp(name, (char *) instance_name, instance_name_len) == 0);
        free(name);
        if (! match) {
            return -3; // name no match
        }
        cid = cid_offset ? GetUShortLE((tag->data + cid_offset)) : 0;
        return cid; // found
    }
    if (! isPlaceTag(tag->code)) {
        fprintf(stderr, "swf_tag_place_get_cid_by_instance_name: ! isPlaceTag(%d)\n", tag->code);
        return -1; // wrong tag
    }
    swf_tag_place = (swf_tag_place_detail_t *) swf_tag_create_input_detail(tag, swf);
    if (swf_tag_place == NULL) {
        fprintf(stderr, "swf_tag_place_get_cid_by_instance_name: swf_tag_place swf_tag_create_input_detail failed\n");
        return -1;
    }
    if ((swf_tag_place->flag_has_name == 0) || (swf_tag_place->name == NULL)) {
        return -2; // no name
    }
    if ((strlen(swf_tag_place->name) != (size_t) instance_name_len) ||
        (strncmp(swf_tag_place->name, (char *) instance_name, (int) instance_name_len) != 0)) {
        return -3; // name no match
    }
    return swf_tag_place->character_id; // found
}
