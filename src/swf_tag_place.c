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
    place_detail_handler.identity = NULL;
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

    if (tag->tag == 4) { // PlaceObject
        swf_tag_place->character_id = bitstream_getbytesLE(bs, 2);
        swf_tag_place->depth = bitstream_getbytesLE(bs, 2);
        ret = swf_matrix_parse(bs, &(swf_tag_place->matrix));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->matrix parse failed. character_id=%d\n", swf_tag_place->character_id);
            bitstream_close(bs);
            return ret;
        }
        ret = swf_cxform_parse(bs, &(swf_tag_place->color_transform));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->color_transform parse failed. character_id=%d\n", swf_tag_place->character_id);
            bitstream_close(bs);
            return ret;
        }
    } else if (tag->tag == 26) { // PlaceObject2
        swf_tag_place->flag_has_clip_action = bitstream_getbit(bs);
        swf_tag_place->flag_has_clip_depth = bitstream_getbit(bs);
        swf_tag_place->flag_has_name = bitstream_getbit(bs);
        swf_tag_place->flag_has_ratio = bitstream_getbit(bs);
        swf_tag_place->flag_has_color_transform = bitstream_getbit(bs);
        swf_tag_place->flag_has_matrix = bitstream_getbit(bs);
        swf_tag_place->flag_has_character = bitstream_getbit(bs);
        swf_tag_place->flag_has_movie = bitstream_getbit(bs);
        swf_tag_place->depth = bitstream_getbytesLE(bs, 2);
        if (swf_tag_place->flag_has_character) {
            swf_tag_place->character_id = bitstream_getbytesLE(bs, 2);
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
            ret = swf_cxform_parse(bs, &(swf_tag_place->color_transform));
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
            swf_tag_place->name = bitstream_outputstring(bs);
        }
        if (swf_tag_place->flag_has_clip_depth) {
            swf_tag_place->clip_depth = bitstream_getbytesLE(bs, 2);   
        }
        // TODO: clip action data for SWF 5
        
    } else {
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

    //
    bs = bitstream_open();
    bitstream_putbytesLE(bs, swf_tag_place->character_id, 2);
/*
    swf_rect_build(bs, &(swf_tag_place->rect));

    // DefineMorphPlace, DefineMorphPlace2
    swf_tag_place->is_morph = (tag->tag == 46) || (tag->tag == 84);
    // DefinePlace4, DefineMorphPlace2
    swf_tag_place->has_strokes = (tag->tag == 83) || (tag->tag == 84);

    if (swf_tag_place->is_morph) {
        ret = swf_rect_build(bs, &(swf_tag_place->rect_morph));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_output_detail: swf_tag_place->rect_morph build failed\n");
            bitstream_close(bs);
            return NULL;
        }
    }
    if (swf_tag_place->has_strokes) {
        ret = swf_rect_build(bs, &(swf_tag_place->stroke_rect));
        if (ret) {
            fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->stroke_rect build failed\n");
            bitstream_close(bs);
            return NULL;
        }
        if (swf_tag_place->is_morph) {
            ret = swf_rect_build(bs, &(swf_tag_place->stroke_rect_morph));
	    if (ret) {
	        fprintf(stderr, "ERROR: swf_tag_place_input_detail: swf_tag_place->stroke_rect_morph build failed\n");
	        bitstream_close(bs);
	        return NULL;
	    }
        }
        bitstream_putbits(bs, 6, swf_tag_place->define_place_reserved );
        bitstream_putbits(bs, 1, swf_tag_place->define_place_non_scaling_strokes );
        bitstream_putbits(bs, 1, swf_tag_place->define_place_scaling_strokes);
    }
    if (swf_tag_place->is_morph) {
        bitstream_putbytesLE(bs, 4, swf_tag_place->offset_morph);
        swf_morph_place_with_style_build(bs, &swf_tag_place->morph_place_with_style, tag);
    } else {
        ret = swf_place_with_style_build(bs, &swf_tag_place->place_with_style, tag);
        if (ret) {
            fprintf(stderr, "swf_tag_place_output_detail: swf_place_with_style_build failed\n");
            bitstream_close(bs);
            return NULL;
        }
    }
*/
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
    if (tag->tag == 4) { // PlaceObject
        printf("character_id=%d  depth=%d\n",
               swf_tag_place->character_id, swf_tag_place->depth);
        swf_matrix_print(&(swf_tag_place->matrix), indent_depth);
        swf_cxform_print(&(swf_tag_place->color_transform), indent_depth);
    } else if (tag->tag == 26) { // PlaceObject2
        printf("(clipact,clipdepth,name,ratio,coltrans,mat,cid,movie)=(%d,%d,%d,%d,%d,%d,%d,%d)\n",
               swf_tag_place->flag_has_clip_action,
               swf_tag_place->flag_has_clip_depth,
               swf_tag_place->flag_has_name,
               swf_tag_place->flag_has_ratio,
               swf_tag_place->flag_has_color_transform,
               swf_tag_place->flag_has_matrix,
               swf_tag_place->flag_has_character,
               swf_tag_place->flag_has_movie);
        if (swf_tag_place->flag_has_character) {
            print_indent(indent_depth);
            printf("character_id=%d\n", swf_tag_place->character_id);
        }
        if (swf_tag_place->flag_has_matrix) {
            swf_matrix_print(&(swf_tag_place->matrix), indent_depth);
        }
        if (swf_tag_place->flag_has_color_transform) {
            swf_cxform_print(&(swf_tag_place->color_transform), indent_depth);
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
        fprintf(stderr, "Illegal tag no(%d)\n", tag->tag);
    }

    /*
    
    print_indent(indent_depth);

    if (swf_tag_place->is_morph) {
        swf_rect_print(&(swf_tag_place->rect_morph), indent_depth);
    }
    if (swf_tag_place->has_strokes) {
        swf_rect_print(&(swf_tag_place->stroke_rect), indent_depth);
        if (swf_tag_place->is_morph) {
            swf_rect_print(&(swf_tag_place->stroke_rect_morph), indent_depth);
        }
        print_indent(indent_depth);
        printf("define_place_non_scaling_strokes=%d define_place_scaling_strokes=%d\n",
               swf_tag_place->define_place_non_scaling_strokes,
               swf_tag_place->define_place_scaling_strokes);
    }
    if (swf_tag_place->is_morph) {
        print_indent(indent_depth);
        printf("offset_morph=%lu\n", swf_tag_place->offset_morph);
        swf_morph_place_with_style_print(&swf_tag_place->morph_place_with_style,
                                         indent_depth, tag);
    } else {
        swf_place_with_style_print(&swf_tag_place->place_with_style,
                                   indent_depth, tag);
    }
    */
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
