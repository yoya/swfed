/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_button_record.h"

/* byte length of the FILTERLIST at data[0..len), or 0 if broken */
unsigned long
swf_filter_list_length(unsigned char *data, unsigned long len) {
    unsigned long pos = 1, i, n;
    if (len < 1) {
        return 0;
    }
    n = data[0]; // NumberOfFilters
    for (i = 0 ; i < n ; i++) {
        unsigned long size;
        if (pos >= len) {
            return 0;
        }
        switch (data[pos]) { // FilterID
          case 0: size = 23; break; // DropShadow
          case 1: size = 9; break;  // Blur
          case 2: size = 15; break; // Glow
          case 3: size = 27; break; // Bevel
          case 4: // GradientGlow
          case 7: // GradientBevel
            if (pos + 1 >= len) {
                return 0;
            }
            size = 1 + 5 * (unsigned long) data[pos + 1] + 19;
            break;
          case 5: // Convolution
            if (pos + 2 >= len) {
                return 0;
            }
            size = 15 + 4 * (unsigned long) data[pos + 1] * data[pos + 2];
            break;
          case 6: size = 80; break; // ColorMatrix
          default:
            return 0;
        }
        pos += 1 + size;
    }
    return (pos <= len) ? pos : 0;
}

int
swf_button_record_parse(bitstream_t *bs, swf_button_record_t *button_record,
    swf_tag_t *tag) {
    int ret;
    ret = bitstream_getbits(bs, 2); // reserved bits, always 0
    if (ret) {
        fprintf(stderr, "swf_button_record_parse: bitstream_getbis(bs, 2)\n");
        return ret;
    }
    button_record->button_has_blend_mode  = bitstream_getbit(bs);
    button_record->button_has_filter_list = bitstream_getbit(bs);
    button_record->button_state_hittest = bitstream_getbit(bs);
    button_record->button_state_down = bitstream_getbit(bs);
    button_record->button_state_over = bitstream_getbit(bs);
    button_record->button_state_up = bitstream_getbit(bs);
    //
    button_record->character_id = bitstream_getbytesLE(bs, 2);
    button_record->place_depth = bitstream_getbytesLE(bs, 2);
    ret = swf_matrix_parse(bs, &(button_record->place_matrix));
    if (ret) {
        fprintf(stderr, "swf_button_record_parse: swf_matrix_parse failed\n");
        return ret;
    }
    if (tag->code == 34) { // DefineButton2
        ret = swf_cxformwithalpha_parse(bs, &(button_record->color_transform));
        if (ret) {
            fprintf(stderr, "swf_button_record_parse: swf_matrix_parse failed\n");
            return ret;
        }
        bitstream_align(bs);
        if (button_record->button_has_filter_list) {
            unsigned long pos = bitstream_getbytepos(bs);
            unsigned long len = swf_filter_list_length(bitstream_buffer(bs, pos),
                                                   bitstream_length(bs) - pos);
            if (len == 0) {
                fprintf(stderr, "swf_button_record_parse: broken filter list\n");
                return 1;
            }
            button_record->filter_list = malloc(len);
            if (button_record->filter_list == NULL) {
                return 1;
            }
            bitstream_getstring(bs, button_record->filter_list, len);
            button_record->filter_list_len = len;
        }
        if (button_record->button_has_blend_mode) {
            button_record->blend_mode = bitstream_getbyte(bs);
        }
    }
    return 0;
}

void
swf_button_record_destroy(swf_button_record_t *button_record) {
    (void) button_record;
    if (button_record) {
        free(button_record->filter_list);
        free(button_record);
    }
    return ;
}

int
swf_button_record_build(bitstream_t *bs, swf_button_record_t *button_record,
    swf_tag_t *tag) {
    int ret;
    bitstream_align(bs); // the previous record may end in the middle of a byte
    bitstream_putbits(bs, 0, 2);
    bitstream_putbit(bs, button_record->button_has_blend_mode);
    bitstream_putbit(bs, button_record->button_has_filter_list);
    bitstream_putbit(bs, button_record->button_state_hittest);
    bitstream_putbit(bs, button_record->button_state_down);
    bitstream_putbit(bs, button_record->button_state_over);
    bitstream_putbit(bs, button_record->button_state_up);
    //
    bitstream_putbytesLE(bs, button_record->character_id, 2);
    bitstream_putbytesLE(bs, button_record->place_depth, 2);
    ret = swf_matrix_build(bs, &(button_record->place_matrix));
    if (ret) {
        fprintf(stderr, "swf_button_record_build: swf_matrix_build failed\n");
        return ret;
    }
    if (tag->code == 34) { // DefineButton2
        ret = swf_cxformwithalpha_build(bs, &(button_record->color_transform));
        if (ret) {
            fprintf(stderr, "swf_button_record_build: swf_matrix_build failed\n");
            return ret;
        }
        bitstream_align(bs);
        if (button_record->button_has_filter_list && button_record->filter_list) {
            bitstream_putstring(bs, button_record->filter_list, button_record->filter_list_len);
        }
        if (button_record->button_has_blend_mode) {
            bitstream_putbyte(bs, button_record->blend_mode);
        }
    }
    return 0;
}

int
swf_button_record_print(swf_button_record_t *button_record, int indent_depth,
    swf_tag_t *tag) {
    print_indent(indent_depth);
    printf("has_blend_mode=%d has_filter_list=%u\n",
           button_record->button_has_blend_mode,
           button_record->button_has_filter_list);
    print_indent(indent_depth);
    printf("state_hittest=%u state_down=%u state_over=%u state_up=%u\n",
           button_record->button_state_hittest,
           button_record->button_state_down,
           button_record->button_state_over,
           button_record->button_state_up);
    print_indent(indent_depth);
    printf("character_id=%d place_depth=%d\n",
           button_record->character_id, button_record->place_depth);
    swf_matrix_print(&(button_record->place_matrix), indent_depth);
    if (tag->code == 34) { // DefineButton2
        swf_cxformwithalpha_print(&(button_record->color_transform), indent_depth);
        print_indent(indent_depth);
        printf("filter_list_len=%lu blend_mode=%d\n",
               button_record->filter_list_len, button_record->blend_mode);
    }
    return 0;
}

/*
 * swf_button_record_list
 */


swf_button_record_list_t *
swf_button_record_list_create(void) {
    swf_button_record_list_t *button_record_list;
    button_record_list = calloc(sizeof(*button_record_list), 1);
    if (button_record_list == NULL) {
        fprintf(stderr, "Can't alloc memory for button_record_list\n");
        return NULL;
    }
    button_record_list->head = NULL;
    button_record_list->tail = NULL;
    return button_record_list;
}

int
swf_button_record_list_parse(bitstream_t *bs, swf_button_record_list_t *button_record_list, swf_tag_t *tag) {
    swf_button_record_t *prev_button_record = NULL;
    int flag;
    while ((flag = bitstream_getbyte(bs)) != 0) { // endflag is 0
        swf_button_record_t *button_record;
        if (flag < 0) {
            /* no end flag. Taking the records read so far as the whole list
             * would drop the rest when the button is written out again, and
             * hide the characters it uses. */
            fprintf(stderr, "swf_button_record_list_parse: unexpected end of stream\n");
            return 1;
        }
        button_record = calloc(1, sizeof(*button_record));
        if (button_record == NULL) {
            return 1;
        }
        button_record->next = NULL;
        bitstream_incrpos(bs, -1, 0); // 1 byte back
        if (swf_button_record_parse(bs, button_record, tag)) {
            fprintf(stderr, "swf_button_record_list_parse: swf_button_record_parse failed\n");
            swf_button_record_destroy(button_record); // also what it has allocated (filter list)
            return 1;
        }
        if (prev_button_record) {
            prev_button_record->next = button_record;
        } else {
            button_record_list->head = button_record;
        }
        prev_button_record = button_record;
    }
    return 0;
}

int
swf_button_record_list_build(bitstream_t *bs, swf_button_record_list_t *button_record_list, swf_tag_t *tag) {
    swf_button_record_t *button_record = NULL;
    for (button_record = button_record_list->head ; button_record ; button_record = button_record->next) {
        swf_button_record_build(bs, button_record, tag);
    }
    bitstream_putbyte(bs, 0); //endflag
    return 0;
}

void
swf_button_record_list_destroy(swf_button_record_list_t *button_record_list) {
    swf_button_record_t *button_record = NULL, *next_button_record = NULL;
    if (button_record_list) {
        for (button_record = button_record_list->head ; button_record ; button_record = next_button_record) {
            next_button_record = button_record->next;
            button_record->next = NULL; // fail safe
            swf_button_record_destroy(button_record);
        }
        free(button_record_list);
    }
}

int
swf_button_record_list_print(swf_button_record_list_t *button_record_list, int indent_depth, swf_tag_t *tag) {
    swf_button_record_t *button_record = NULL;
    for (button_record = button_record_list->head ; button_record ; button_record = button_record->next) {
        swf_button_record_print(button_record, indent_depth, tag);
    }
    return 0;
}

