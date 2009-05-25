#include <stdio.h>
#include "bitstream.h"
#include "swf_rgba.h"

int
swf_rgba_parse(bitstream_t *bs, swf_rgba_t *color) {
    color->red   = bitstream_getbyte(bs);
    color->green = bitstream_getbyte(bs);
    color->blue  = bitstream_getbyte(bs);
    color->alpha = bitstream_getbyte(bs);
    return 0;
}

int
swf_rgba_build(bitstream_t *bs, swf_rgba_t *color) {
    bitstream_putbyte(bs, color->red);
    bitstream_putbyte(bs, color->green);
    bitstream_putbyte(bs, color->blue);
    bitstream_putbyte(bs, color->alpha);
    return 0;
}

int
swf_rgba_print(swf_rgba_t *color, int indent_depth) {
    print_indent(indent_depth);
    printf("red=0x%02x  green=0x%02X  blue=0x%02x  alpha=0x%02x\n",
           color->red, color->green, color->blue, color->alpha);
    return 0;
}
