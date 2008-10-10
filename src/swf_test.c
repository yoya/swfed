#include <stdio.h>
#include <sys/stat.h>
#include "swf_object.h"

typedef struct  _binedit_string {
    unsigned char *ptr;
    unsigned long len;
} binedit_string_t;

int loadfile(binedit_string_t *str, char *filename) {
    FILE *fp;
    struct stat sbuf;
    if (stat(filename, &sbuf)) {
        return 1;
    }
    str->len = sbuf.st_size;
    str->ptr = calloc(str->len, 1);
    fp = fopen(filename, "rb");
    if (fread(str->ptr, 1, str->len, fp) != str->len) {
        return 1;
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv) {
  swf_object_t *swf;
  binedit_string_t str;
  loadfile(&str, "saitama.swf");
  swf = swf_object_open();
  swf = swf_object_set(str.ptr, str.len);
  return 0;
}
