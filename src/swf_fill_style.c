#include <stdio.h>
#include "bitstream.h"
#include "swf_fill_style.h"

int
swf_fill_style_parse(bitstream_t *bs,
                     swf_fill_style_t *shape_with_style, swf_tag_t * tag) {
    shape_with_style->type = bitstream_getbyte(bs);
    switch (shape_with_style->type) {
      case 0x00: // solid fill
//        swf_fill_style_solid_parse(bs, shape_with_style, tag);
        break;
      case 0x10: // linear gradientfill
      case 0x11: // radial gradientfill
      case 0x12: // focal gradient fill
//        swf_fill_style_gradient_parse(bs, shape_with_style, tag);
        break;
      case 0x40: // tilled bitmap fill with smoothed edges
      case 0x41: // clipped bitmap fill with smoothed edges
      case 0x42: // tilled bitmap fill with hard edges
      case 0x43: // clipped bitmap fill with hard edges
        //        swf_fill_style_bitmap_parse(bs, shape_with_style, tag);
        break;
    }
    return 0;
}

int
swf_fill_style_build(bitstream_t *bs, swf_fill_style_t *shape_with_style) {
    return 0;
}

int
swf_fill_style_print(swf_fill_style_t *shape_with_style, int indent_depth) {
    print_indent(indent_depth);
    printf("type=0x%02x\n", shape_with_style->type);
        switch (shape_with_style->type) {
          case 0x00: // solid fill
//        swf_fill_style_solid_print(bs, shape_with_style, indent_depth);
              break;
          case 0x10: // linear gradientfill
          case 0x11: // radial gradientfill
          case 0x12: // focal gradient fill
//        swf_fill_style_gradient_print(hape_with_style, indent_depth);
            break;
          case 0x40: // tilled bitmap fill with smoothed edges
          case 0x41: // clipped bitmap fill with smoothed edges
          case 0x42: // tilled bitmap fill with hard edges
          case 0x43: // clipped bitmap fill with hard edges
//        swf_fill_style_bitmap_print(shape_with_style, indent_depth);
            break;
        }
    return 0;
}
