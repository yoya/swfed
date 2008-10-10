#include <stdio.h>
#include "bitstream.h"
#include "swf_xrgb.h"

int
swf_xrgb_parse(bitstream_t *bs, swf_xrgb_t *color) {
    (void) bitstream_getbyte(bs); // pad
    color->red   = bitstream_getbyte(bs);
    color->green = bitstream_getbyte(bs);
    color->blue  = bitstream_getbyte(bs);
    return 0;
}

int
swf_xrgb_build(bitstream_t *bs, swf_xrgb_t *color) {
    bitstream_putbyte(bs, 0); // pad
    bitstream_putbyte(bs, color->red);
    bitstream_putbyte(bs, color->green);
    bitstream_putbyte(bs, color->blue);
    return 0;
}

int
swf_xrgb_print(swf_xrgb_t *color) {
    printf("pad=0xXX red=0x%02x  green=0x%02X  blue=0x%02x\n",
           color->red, color->green, color->blue);
    return 0;
}
