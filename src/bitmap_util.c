/*
  gcc -W -Wall -D__BITMAP_UTIL_DEBUG__ bitmap_util.c jpeg_segment.c bitstream.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_segment.h"
#include "bitmap_util.h"

#define isJPEG_DHT(c) ((c) == 0xC4)
#define isJPEG_JPG(c) ((c) == 0xC8)
#define isJPEG_DAC(c) ((c) == 0xCC)

#define isJPEG_SOFXX(c) ((0xC0 <= c) && (c <= 0xCF) && (!isJPEG_DHT(c)) && (!isJPEG_JPG(c)) && (!isJPEG_DAC(c)))

int jpeg_size_segment(jpeg_segment_t *jpeg_seg, int *width, int *height) {
    jpeg_segment_node_t *node;
    for (node = jpeg_seg->head ; node ; node = node->next) {
        register int marker = node->marker;
        if (isJPEG_SOFXX(marker)) {
            *width  = 0x100 * node->data_ref[3] + node->data_ref[4];
            *height = 0x100 * node->data_ref[1] + node->data_ref[2];
            return 0; // found
        }
    }
    return 1; // not found
}

int jpeg_size(unsigned char *data, unsigned long data_len,
              int *width, int *height) {
    int ret = 0;
    jpeg_segment_t *jpeg_seg = NULL;
    jpeg_seg = jpeg_segment_parse(data, data_len, 0);
    ret = jpeg_size_segment(jpeg_seg, width, height);
    jpeg_segment_destroy(jpeg_seg);
    return ret;
}

#ifdef __BITMAP_UTIL_DEBUG__  /* for component debug */

#include <sys/stat.h>

int main(int argc, char **argv) {
    char *filename;
    struct stat sbuf;
    FILE *fp = NULL;
    unsigned char *data;
    unsigned long data_len;
    int width = 0, height = 0;
    int ret;
    int i;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <jpeg_file> [<jpeg_file2> [...]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (i = 1 ; i < argc ; i++) {
        filename = argv[i];
        if (stat(filename, &sbuf)) {
            fprintf(stderr, "Can't stat file(%s)\n", filename);
            return EXIT_FAILURE;
        }
        data_len = sbuf.st_size;
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            fprintf(stderr, "Can't open infile(%s)\n", filename);
            return EXIT_FAILURE;
        }
        data = malloc(data_len);
        if (fread(data, 1, data_len, fp) != data_len) {
            fclose(fp);
            return 1;
        }
        fclose(fp);
        ret = jpeg_size(data, data_len, &width, &height);
        free(data);
        if (ret) {
            fprintf(stderr, "Can't get jpeg size(%s)\n", filename);
            return EXIT_FAILURE;
        }
        printf("width=%d height=%d\n", width, height);
    }
    return EXIT_SUCCESS;
}

int
png_size(unsigned char *data, unsigned long data_len,
         int *width, int *height) {
    return 0;
}

int
gif_size(unsigned char *data, unsigned long data_len,
                    int *width, int *height) {
    return 0;
}


#endif /* __BITMAP_UTIL_DEBUG__ */
