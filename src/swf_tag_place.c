/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_tag_place.h"
#include "swf_action.h"
#include "swf_object.h"

static const swf_tag_detail_handler_t swf_tag_place_detail_handler_table = {
    .create      = swf_tag_place_create_detail,
    .input       = swf_tag_place_input_detail,
    .get_cid     = NULL,
    .replace_cid = NULL,
    .output      = swf_tag_place_output_detail,
    .print       = swf_tag_place_print_detail,
    .destroy     = swf_tag_place_destroy_detail,
};

const swf_tag_detail_handler_t *
swf_tag_place_detail_handler(void) {
    return &swf_tag_place_detail_handler_table;
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
        if (swf_tag_place->flag_has_clip_action) {
            unsigned long pos;
            bitstream_align(bs);
            pos = bitstream_getbytepos(bs);
            if (pos >= length) {
                fprintf(stderr, "ERROR: swf_tag_place_input_detail: no clip actions. character_id=%d\n", swf_tag_place->character_id);
                bitstream_close(bs);
                return 1;
            }
            swf_tag_place->clip_actions_len = length - pos;
            swf_tag_place->clip_actions = malloc(swf_tag_place->clip_actions_len);
            if (swf_tag_place->clip_actions == NULL) {
                bitstream_close(bs);
                return 1;
            }
            memcpy(swf_tag_place->clip_actions, data + pos, swf_tag_place->clip_actions_len);
        }
        
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
        if (swf_tag_place->flag_has_clip_action && swf_tag_place->clip_actions) {
            bitstream_align(bs);
            bitstream_putstring(bs, swf_tag_place->clip_actions, swf_tag_place->clip_actions_len);
        }
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
        free(swf_tag_place->clip_actions);
        free(swf_tag_place);
        tag->detail = NULL;
    }
    return ;
}

int
swf_tag_place_get_cid_by_instance_name(swf_tag_t *tag, unsigned char *instance_name, int instance_name_len, struct swf_object_ *swf) {
    swf_tag_place_detail_t *swf_tag_place;
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

static unsigned long
_get_le(unsigned char *d, int size) {
    unsigned long v = 0;
    int i;
    for (i = 0 ; i < size ; i++) {
        v |= (unsigned long) d[i] << (8 * i);
    }
    return v;
}

/*
 * Replace strings in the actions of CLIPACTIONS (onClipEvent), which are
 * kept as raw bytes. EventFlags are UI16 up to SWF 5 and UI32 from SWF 6.
 */
int
swf_tag_place_replace_action_strings(swf_tag_t *tag, y_keyvalue_t *kv,
                                     int *modified, int swf_version) {
    swf_tag_place_detail_t *swf_tag_place = (swf_tag_place_detail_t *) tag->detail;
    int flag_size = (swf_version >= 6) ? 4 : 2;
    unsigned char *d;
    unsigned long len, pos;
    bitstream_t *out;
    int changed = 0;
    *modified = 0;
    if ((swf_tag_place == NULL) || (swf_tag_place->clip_actions == NULL)) {
        return 0;
    }
    d = swf_tag_place->clip_actions;
    len = swf_tag_place->clip_actions_len;
    if (len < (unsigned long) (2 + flag_size)) {
        return 1;
    }
    out = bitstream_open();
    bitstream_putstring(out, d, 2 + flag_size); // Reserved, AllEventFlags
    pos = 2 + flag_size;
    while (1) {
        unsigned long size, keycode_len;
        unsigned char *actions;
        swf_action_list_t *list;
        bitstream_t *bs;
        int m = 0;
        if (pos + flag_size > len) {
            goto fail;
        }
        if (_get_le(d + pos, flag_size) == 0) { // ClipActionEndFlag
            bitstream_putstring(out, d + pos, flag_size);
            break;
        }
        if (pos + flag_size + 4 > len) {
            goto fail;
        }
        size = _get_le(d + pos + flag_size, 4); // ActionRecordSize
        if (size > len - (pos + flag_size + 4)) {
            goto fail;
        }
        // KeyCode follows when ClipEventKeyPress (SWF 6+) is set
        keycode_len = ((flag_size == 4) && (d[pos + 2] & 0x02)) ? 1 : 0;
        if (size < keycode_len) {
            goto fail;
        }
        actions = d + pos + flag_size + 4 + keycode_len;
        list = swf_action_list_create();
        bs = bitstream_open();
        bitstream_input(bs, actions, size - keycode_len);
        if ((list == NULL) || swf_action_list_parse(bs, list) ||
            swf_action_list_replace_strings(list, &m, kv)) {
            bitstream_close(bs);
            swf_action_list_destroy(list);
            goto fail;
        }
        bitstream_close(bs);
        bitstream_putstring(out, d + pos, flag_size); // EventFlags
        if (m) {
            unsigned long actions_len;
            unsigned char *new_actions;
            bs = bitstream_open();
            swf_action_list_build(bs, list);
            new_actions = bitstream_steal(bs, &actions_len);
            bitstream_close(bs);
            bitstream_putbytesLE(out, keycode_len + actions_len, 4);
            bitstream_putstring(out, d + pos + flag_size + 4, keycode_len);
            bitstream_putstring(out, new_actions, actions_len);
            free(new_actions);
            changed = 1;
        } else {
            bitstream_putstring(out, d + pos + flag_size, 4 + size);
        }
        swf_action_list_destroy(list);
        pos += flag_size + 4 + size;
    }
    if (changed) {
        unsigned long new_len;
        free(swf_tag_place->clip_actions);
        swf_tag_place->clip_actions = bitstream_steal(out, &new_len);
        swf_tag_place->clip_actions_len = new_len;
        *modified = 1;
    }
    bitstream_close(out);
    return 0;
 fail:
    fprintf(stderr, "swf_tag_place_replace_action_strings: broken clip actions\n");
    bitstream_close(out);
    return 1;
}
