/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swf_define.h"

#ifdef HAVE_GIF

#include <gif_lib.h>
#include "bitstream.h"
#include "swf_rgb.h"  // Lossless  format=3
#include "swf_rgba.h" // Lossless2 format=3
#include "swf_xrgb.h" // Lossless  format=5
#include "swf_argb.h" // Lossless2 format=5
#include "swf_gif.h"

typedef struct my_gif_buffer_ {
    unsigned char *data;
    unsigned long data_len;
    unsigned long data_offset;
} my_gif_buffer;

typedef unsigned char gif_byte;
typedef gif_byte *    gif_bytep;
typedef gif_bytep *   gif_bytepp;
typedef unsigned long gif_uint_32;
typedef struct {
    unsigned char red, green, blue;
    unsigned char alpha;
} gif_color;
typedef gif_color *   gif_colorp;

/*
 * gif read
 */

int
gif_data_read_func(GifFileType* GifFile, GifByteType* buf, int count) {
    my_gif_buffer *gif_buff = (my_gif_buffer *) GifFile->UserData;
    if (gif_buff->data_offset + count <= gif_buff->data_len) {
        memcpy(buf, gif_buff->data + gif_buff->data_offset, count);
        gif_buff->data_offset += count;
    } else {
        fprintf(stderr, "! gif_buff->data_offset(%lu) + count(%d) <= gif_buff->data_len(%lu)\n",
                gif_buff->data_offset, count, gif_buff->data_len);
        return 0;
    }
    return count;
}

/*
 * What decoding the GIF allocates, from the sizes written in it: the logical
 * screen (rows aligned to 4 bytes, for the bitmap) and every frame (giflib).
 * It follows the blocks as giflib does and stops at the first thing it can't
 * follow, where giflib fails too.
 */
static unsigned long
gif_decoded_size(const unsigned char *d, unsigned long len) {
    unsigned long pos, total, w, h;
    if (len < 13) {
        return 0;
    }
    w = d[6] | (d[7] << 8);
    h = d[8] | (d[9] << 8);
    total = ((w + 3) & ~3UL) * h;
    pos = 13;
    if (d[10] & 0x80) { // global color table
        pos += 3UL << ((d[10] & 0x07) + 1);
    }
    while (pos < len) {
        int c = d[pos++];
        if (c == 0x2C) { // image descriptor: left, top, width, height, flags
            unsigned long frame;
            if (len - pos < 9) {
                break;
            }
            frame = (unsigned long) (d[pos + 4] | (d[pos + 5] << 8)) *
                (unsigned long) (d[pos + 6] | (d[pos + 7] << 8));
            if (total + frame < total) {
                return (unsigned long) -1;
            }
            total += frame;
            if (d[pos + 8] & 0x80) { // local color table
                pos += 3UL << ((d[pos + 8] & 0x07) + 1);
            }
            pos += 9 + 1; // + LZW minimum code size
        } else if (c == 0x21) { // extension: label
            pos++;
        } else { // trailer, or unknown
            break;
        }
        while ((pos < len) && d[pos]) { // data sub-blocks
            pos += 1 + (unsigned long) d[pos];
        }
        pos++; // block terminator
    }
    return total;
}

/*
 * getTransparentIndex
 */
int
getTransparentIndex(SavedImage Image) {
    int i;
    int transparent_index = -1;
    if ((Image.ExtensionBlockCount == 0) || (Image.ExtensionBlocks == NULL)) {
        return -1;
    }
    for (i = 0 ; i < Image.ExtensionBlockCount ; i++ ) {
        ExtensionBlock Block = Image.ExtensionBlocks[i];
        if ((Block.Function == GRAPHICS_EXT_FUNC_CODE) &&
            (Block.ByteCount >= 4) && (Block.Bytes != NULL)) {
            int gcntl_flag = Block.Bytes[0];
            if (gcntl_flag & 0x01) {
                transparent_index = Block.Bytes[3] & 0xff;
                return transparent_index;
            }
        }
    }
    return transparent_index;
}

/*
 *
 */

void *
gifconv_gif2lossless(unsigned char *gif_data, unsigned long gif_data_len,
                     int *tag_no, int *format,
                     unsigned short *width, unsigned short *height,
                     void **colormap, int *colormap_count) {
    GifFileType *GifFile = NULL;
    ColorMapObject *ColorMap = NULL;
    my_gif_buffer gif_buff;
    int bpp;
    gif_uint_32 gif_width = 0, gif_height = 0;
    gif_bytep gif_image_data_ref = NULL;
    gif_uint_32 x, y;

    void *image_data = NULL;
    int palette_num = 0;
    int trans_index = -1;
    SavedImage Image;
    int i;
    unsigned char *indices_data;

    if (swf_size_limit_exceeded("gifconv_gif2lossless",
                                gif_decoded_size(gif_data, gif_data_len))) {
        return NULL;
    }
    gif_buff.data = gif_data;
    gif_buff.data_len = gif_data_len;
    gif_buff.data_offset = 0;
#if GIFLIB_MAJOR >= 5
    GifFile = DGifOpen(& gif_buff, gif_data_read_func, NULL);
#else
    GifFile = DGifOpen(& gif_buff, gif_data_read_func);
#endif
    if (GifFile == NULL) {
        fprintf(stderr, "gifconv_gif2lossless: can't open GIFFile\n");
        return NULL;
    }
    if (DGifSlurp(GifFile) == GIF_ERROR) {
        fprintf(stderr, "gifconv_gif2lossless: DGifSlurp failed\n");
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
        DGifCloseFile(GifFile, NULL);
#else
        DGifCloseFile(GifFile);
#endif
        return NULL;
    }
    if ((GifFile->ImageCount < 1) || (GifFile->SavedImages == NULL) ||
        (GifFile->SavedImages[0].RasterBits == NULL)) {
        fprintf(stderr, "gifconv_gif2lossless: no image in GIF\n");
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
        DGifCloseFile(GifFile, NULL);
#else
        DGifCloseFile(GifFile);
#endif
        return NULL;
    }
    Image = GifFile->SavedImages[0];
    ColorMap = Image.ImageDesc.ColorMap;  // local color table has priority
    if (ColorMap == NULL) {
        ColorMap = GifFile->SColorMap;
    }
    gif_width  = GifFile->SWidth;
    gif_height = GifFile->SHeight;
    if ((ColorMap == NULL) || (ColorMap->Colors == NULL) ||
        (gif_width == 0) || (gif_height == 0) ||
        (gif_width > 0xFFFF) || (gif_height > 0xFFFF) ||
        (Image.ImageDesc.Width <= 0) || (Image.ImageDesc.Height <= 0)) {
        fprintf(stderr, "gifconv_gif2lossless: invalid GIF (no colormap or bad size)\n");
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
        DGifCloseFile(GifFile, NULL);
#else
        DGifCloseFile(GifFile);
#endif
        return NULL;
    }
    bpp = ColorMap->BitsPerPixel;
    if (bpp > 8) {
        fprintf(stderr, "gifconv_gif2lossless: bpp=%d not implemented. accept only bpp <= 8\n", bpp);
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
        DGifCloseFile(GifFile, NULL);
#else
        DGifCloseFile(GifFile);
#endif
        return NULL;
    }
    palette_num = ColorMap->ColorCount;
    if ((palette_num <= 0) || (palette_num > 256)) {
        fprintf(stderr, "gifconv_gif2lossless: invalid palette_num=%d\n", palette_num);
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
        DGifCloseFile(GifFile, NULL);
#else
        DGifCloseFile(GifFile);
#endif
        return NULL;
    }

    trans_index = getTransparentIndex(Image);

    *width  = (unsigned short) gif_width;
    *height = (unsigned short) gif_height;
    *format = 3;

    if (trans_index != -1) {
        *tag_no = 36; // DefineBitsLossless2
    } else {
        *tag_no = 20; // DefineBitsLossless
    }

    /*
     * image copy
     */
    *colormap_count = palette_num;
    if (trans_index == -1) {  // Lossless
        swf_rgb_t *result_colormap = malloc(sizeof(swf_rgb_t) * palette_num);
        if (result_colormap == NULL) {
            goto fail;
        }
        for (i=0 ; i < palette_num ; i++) {
            result_colormap[i].red   = ColorMap->Colors[i].Red;
            result_colormap[i].green = ColorMap->Colors[i].Green;
            result_colormap[i].blue  = ColorMap->Colors[i].Blue;
        }
        *colormap = result_colormap;
    } else {  // Lossless2
        swf_rgba_t *result_colormap = malloc(sizeof(swf_rgba_t) * palette_num);
        if (result_colormap == NULL) {
            goto fail;
        }
        for (i=0 ; i < palette_num ; i++) {
            if (i == trans_index) {
                result_colormap[i].red   = 0x0;
                result_colormap[i].green = 0x0;
                result_colormap[i].blue  = 0x0;
                result_colormap[i].alpha = 0x0;
            } else {
                result_colormap[i].red   = ColorMap->Colors[i].Red;
                result_colormap[i].green = ColorMap->Colors[i].Green;
                result_colormap[i].blue  = ColorMap->Colors[i].Blue;
                result_colormap[i].alpha = 0xff;
            }
        }
        *colormap = result_colormap;
    }
    {
        /* the frame may be smaller than / offset within the logical screen */
        unsigned long stride = ((unsigned long) gif_width + 3) & ~3UL;
        long frame_left   = Image.ImageDesc.Left;
        long frame_top    = Image.ImageDesc.Top;
        long frame_width  = Image.ImageDesc.Width;
        long frame_height = Image.ImageDesc.Height;
        int fill_index = (trans_index != -1) ? trans_index : GifFile->SBackGroundColor;
        if ((fill_index < 0) || (fill_index >= palette_num)) {
            fill_index = 0;
        }
        indices_data = malloc(stride * gif_height);
        if (indices_data == NULL) {
            free(*colormap);
            *colormap = NULL;
            goto fail;
        }
        memset(indices_data, fill_index, stride * gif_height);
        gif_image_data_ref = Image.RasterBits;
        for (y=0 ; y < gif_height ; y++) {
            long fy = (long) y - frame_top;
            if ((fy < 0) || (fy >= frame_height)) {
                continue;
            }
            for (x=0 ; x < gif_width ; x++) {
                long fx = (long) x - frame_left;
                int index;
                if ((fx < 0) || (fx >= frame_width)) {
                    continue;
                }
                index = gif_image_data_ref[fx + fy * frame_width];
                if (index >= palette_num) {
                    fprintf(stderr, "gifconv_gif2lossless: color index(%d) >= palette_num(%d)\n",
                            index, palette_num);
                    free(indices_data);
                    free(*colormap);
                    *colormap = NULL;
                    goto fail;
                }
                indices_data[x + y * stride] = index;
            }
        }
    }
    image_data = indices_data;
    /*
     * destruct
     */
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
    DGifCloseFile(GifFile, NULL);
#else
    DGifCloseFile(GifFile);
#endif
    return image_data;
fail:
#if GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1 || GIFLIB_MAJOR > 5
    DGifCloseFile(GifFile, NULL);
#else
    DGifCloseFile(GifFile);
#endif
    return NULL;
}

#endif /* HAVE_GIF */
