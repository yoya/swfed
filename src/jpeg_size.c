/*
  gcc -W -Wall -D__JPEG_SIZE_DEBUG__ jpeg_size.c jpeg_segment.c  bitstream.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "jpeg_segment.h"

int jpeg_size(unsigned char *data, unsigned long data_len,
              int *width, int *height) {
    jpeg_segment_t *jpeg_seg;
    jpeg_segment_node_t *node;
    jpeg_seg = jpeg_segment_parse(data, data_len, 0);
    for (node = jpeg_seg->head ; node ; node = node->next) {
        if ((0xC0 <= node->marker) && (node->marker <= 0xCF)) { // SOF
            *width  = 0x100 * node->data_ref[3] + node->data_ref[4];
            *height = 0x100 * node->data_ref[1] + node->data_ref[2];
            jpeg_segment_destroy(jpeg_seg);
            return 0;
        }
    }
    jpeg_segment_destroy(jpeg_seg);
    return 1;
}

#ifdef __JPEG_SIZE_DEBUG__  /* for component debug */

#include <sys/stat.h>

int main(int argc, char **argv) {
    char *filename;
    struct stat sbuf;
    FILE *fp = NULL;
    unsigned char *data;
    unsigned long data_len;
    int width = 0, height = 0;
    int ret;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <jpeg_infile>\n", argv[0]);
        return EXIT_FAILURE;
    }
    filename = argv[1];
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
    return EXIT_SUCCESS;
}

#endif /* __JPEG_SIZE_DEBUG__ */
