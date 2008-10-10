#include <stdio.h>
#include "bitstream.h"
#include "swf_rect.h"


int two_negative(int num, int size) {
    int msb = 1 << (size - 1);
    int mask = msb - 1;
    if (num & msb) {
        return - ((num^mask) & mask) - 1;
    }
    return num;
}

int two_negative_reverse(int num, int size) { // dummy 
    int msb = 1 << (size - 1);
    int mask = msb - 1;
    if (num < 0) {
        return - ((num^mask) & mask) - 1;
    }
    return num;
}

int
swf_rect_parse(bitstream_t *bs, swf_rect_t *rect) {
    unsigned char size;
    bitstream_align(bs);
    size = bitstream_getbits(bs, 5);
    rect->size = size;
    rect->x_min = bitstream_getbits(bs, size);
    rect->x_max = bitstream_getbits(bs, size);
    rect->y_min = bitstream_getbits(bs, size);
    rect->y_max = bitstream_getbits(bs, size);
    //
    rect->x_min = two_negative(rect->x_min, size);
    rect->x_max = two_negative(rect->x_max, size);
    rect->y_min = two_negative(rect->y_min, size);
    rect->y_max = two_negative(rect->y_max, size);
    return 0;
}

int
swf_rect_build(bitstream_t *bs, swf_rect_t *rect) {
    unsigned char size = rect->size; // XXX
    int x_min = two_negative_reverse(rect->x_min, size);
    int x_max = two_negative_reverse(rect->x_max, size);
    int y_min = two_negative_reverse(rect->y_min, size);
    int y_max = two_negative_reverse(rect->y_max, size);
    bitstream_align(bs);
    bitstream_putbits(bs, size, 5);
    bitstream_putbits(bs, x_min, size);
    bitstream_putbits(bs, x_max, size);
    bitstream_putbits(bs, y_min, size);
    bitstream_putbits(bs, y_max, size);
    return 0;
}

int
swf_rect_print(swf_rect_t *rect) {
    printf("rect=(%d, %d)-(%d, %d) (f_size=%d)\n",
           rect->x_min / SWF_TWIPS,
           rect->y_min / SWF_TWIPS,
           rect->x_max / SWF_TWIPS,
           rect->y_max / SWF_TWIPS,
           rect->size);
    return 0;
}
