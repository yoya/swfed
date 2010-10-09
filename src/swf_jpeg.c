/*
  +----------------------------------------------------------------------+
  | Author: yoya@awm.jp                                                  |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#include <stdlib.h>
#include "bitstream.h"
#include "swf_jpeg.h"
#include "jpeg_segment.h"

unsigned char *
jpegconv_std2swf(unsigned char *data, unsigned long data_len,
                 unsigned long *length) {
    jpeg_segment_t *jpeg_data = NULL;
    jpeg_segment_t *jpeg_output;
    unsigned char *d;
    unsigned long d_len;
    int m;
    unsigned char *ret_data;
    jpeg_data = jpeg_segment_parse(data, data_len, 1);
    if (jpeg_data == NULL) {
        fprintf(stderr, "jpegconv_std2swf: Can't create jpeg segment for data\n");
        return NULL;
    }
    jpeg_output = jpeg_segment_create();
    if (jpeg_output == NULL) {
        fprintf(stderr,
                "jpegconv_std2swf: Can't create jpeg segment for output\n");
        jpeg_segment_destroy(jpeg_data);
        return NULL;
    }
    /* 並び替え */
    // 圧縮テーブル部
    jpeg_segment_append(jpeg_output, 0xD8, NULL, 0); // SOI
    while ((d = jpeg_segment_steal_node(jpeg_data, 0xDB, &d_len)) != NULL) {
        jpeg_segment_append(jpeg_output, 0xDB, d, d_len); // DQT
    }
    while ((d = jpeg_segment_steal_node(jpeg_data, 0xC4, &d_len)) != NULL) {
        jpeg_segment_append(jpeg_output, 0xC4, d, d_len); // DHT
    }
    jpeg_segment_append(jpeg_output, 0xD9, NULL, 0); // EOI
    // 画像実データ部 (APP 等も含む)
    while((m = jpeg_segment_peek_marker(jpeg_data)) >= 0) {
        d = jpeg_segment_steal_node(jpeg_data, m, &d_len);
        jpeg_segment_append(jpeg_output, m, d, d_len);
    }
    ret_data = jpeg_segment_output(jpeg_output, length);
    jpeg_segment_destroy(jpeg_data);
    jpeg_segment_destroy(jpeg_output);
    return ret_data;
}

unsigned char *
jpegconv_swf2std(unsigned char *data, unsigned long data_len,
                 unsigned long *length, unsigned char *table_data,
                 unsigned long table_data_len) {
    jpeg_segment_t *jpeg_data, *jpeg_table = NULL;
    jpeg_segment_t *jpeg_output;
    int m;
    unsigned char *d;
    unsigned long d_len;
    unsigned char *ret_data;
    *length = 0;
    jpeg_data = jpeg_segment_parse(data, data_len, 1);
    if (jpeg_data == NULL) {
            fprintf(stderr, "jpegconv_swf2std: Can't create jpeg segment for data\n");
            return NULL;
    }
    if (jpeg_segment_contain(jpeg_data, 0xDB)) {
        /* 圧縮テーブルが含まれている場合 */
        jpeg_table = jpeg_segment_create();
        if (jpeg_table == NULL) {
            fprintf(stderr,
                    "jpegconv_swf2std: Can't create jpeg segment for table\n");
            jpeg_segment_destroy(jpeg_data);
            return NULL;
        }
        while ((d = jpeg_segment_steal_node(jpeg_data, 0xDB, &d_len)) != NULL) {
            jpeg_segment_append(jpeg_table, 0xDB, d, d_len);  // DQT
        }
        while ((d = jpeg_segment_steal_node(jpeg_data, 0xC4, &d_len)) != NULL) {
            jpeg_segment_append(jpeg_table, 0xC4, d, d_len); // DHT
        }
    } else if (table_data && table_data_len) {
        /* 圧縮テーブルが入っていない場合は JPEGTables を参照 */
        jpeg_table = jpeg_segment_parse(table_data, table_data_len, 1);
        if (jpeg_table == NULL) {
            fprintf(stderr, "Can't create jpeg segment for table\n");
            jpeg_segment_destroy(jpeg_data);
            return NULL;
        }
    } else {
        fprintf(stderr, "jpegconv_swf2std: not found jpeg table segment\n");
        jpeg_segment_destroy(jpeg_data);
        return NULL;
    }
    /* 並び替え */
    jpeg_segment_delete_node(jpeg_data, 0xD8); // SOI
    jpeg_segment_delete_node(jpeg_data, 0xD9); // EOI
    jpeg_output = jpeg_segment_create();
    if (jpeg_output == NULL) {
        fprintf(stderr, "jpegconv_swf2std: Can't create jpeg segment for output\n");
        jpeg_segment_destroy(jpeg_data);
        jpeg_segment_destroy(jpeg_table);
        return NULL;
    }
    jpeg_segment_append(jpeg_output, 0xD8, NULL, 0); // SOI
    /* 圧縮テーブルを退避 */
    /* SOF0,1,2 マーカーの手前まで構成 */
    while((m = jpeg_segment_peek_marker(jpeg_data)) >= 0) {
        if ((m == 0xC0) ||  (m == 0xC1) || (m == 0xC2)) { // SOF0,1,2
            break;
        }
        d = jpeg_segment_steal_node(jpeg_data, m, &d_len);
        jpeg_segment_append(jpeg_output, m, d, d_len);
    };
    while ((d = jpeg_segment_steal_node(jpeg_table, 0xDB, &d_len)) != NULL) {
        jpeg_segment_append(jpeg_output, 0xDB, d, d_len); // DQT
    }
    /* SOF0,1,2 を構成 */
    while((m = jpeg_segment_peek_marker(jpeg_data)) >= 0) {
        if ((m != 0xC0) &&  (m != 0xC1) && (m != 0xC2)) { // SOF0,1,2
            break;
        }
        d = jpeg_segment_steal_node(jpeg_data, m, &d_len);
        jpeg_segment_append(jpeg_output, m, d, d_len);
    }
    while ((d = jpeg_segment_steal_node(jpeg_table, 0xC4, &d_len)) != NULL) {
        jpeg_segment_append(jpeg_output, 0xC4, d, d_len); // DHT
    }
    while((m = jpeg_segment_peek_marker(jpeg_data)) >= 0) {
        d = jpeg_segment_steal_node(jpeg_data, m, &d_len);
        jpeg_segment_append(jpeg_output, m, d, d_len);
    }
    jpeg_segment_append(jpeg_output, 0xD9, NULL, 0); // EOI
    ret_data = jpeg_segment_output(jpeg_output, length);
    jpeg_segment_destroy(jpeg_data);
    jpeg_segment_destroy(jpeg_table);
    jpeg_segment_destroy(jpeg_output);
    return ret_data;
}
