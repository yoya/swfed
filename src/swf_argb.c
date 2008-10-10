#include <stdio.h>
#include "bitstream.h"
#include "swf_argb.h"

int
swf_argb_parse(bitstream_t *bs, swf_argb_t *color) {
    color->alpha = bitstream_getbyte(bs);
    color->red   = bitstream_getbyte(bs);
    color->green = bitstream_getbyte(bs);
    color->blue  = bitstream_getbyte(bs);
    return 0;
}

int
swf_argb_build(bitstream_t *bs, swf_argb_t *color) {
    bitstream_putbyte(bs, color->alpha);
    bitstream_putbyte(bs, color->red);
    bitstream_putbyte(bs, color->green);
    bitstream_putbyte(bs, color->blue);
    return 0;
}

int
swf_argb_print(swf_argb_t *color) {
    printf("alpha=0x%02x  red=0x%02x  green=0x%02X  blue=0x%02x\n",
           color->alpha, color->red, color->green, color->blue);
    return 0;
}
