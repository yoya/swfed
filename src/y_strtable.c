#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "swf_define.h"
#include "y_strtable.h"

#define Y_STRTABLE_INITSIZE 10

y_strtable_t *
y_strtable_open() {
    int i;
    y_strtable_t *st = calloc(sizeof(*st), 1);
    if (st == NULL) {
        fprintf(stderr, "y_strtable_open: calloc failed\n");
        return NULL;
    }
    st->use_len = 0;
    st->alloc_len = Y_STRTABLE_INITSIZE;
    st->table = malloc(sizeof(st->table) * Y_STRTABLE_INITSIZE);
    return st;
}

int
y_strtable_set(y_strtable_t *st, char *key, int key_len, char *value, int value_len) {
    int i, offset = -1;
    void *tmp;
    if (st->use_len < st->alloc_len) {
        offset = st->use_len;
    } else {
        for (i = 0 ; i < st->alloc_len ; i++) {
            if (st->table[i].use == 0) {
                offset = i;
                break;
            }
        }
        if (offset == -1) {
            tmp = realloc(st->table, 2 * st->alloc_len);
            if (tmp == NULL) {
                fprintf(stderr, "y_strtable_set: realloc failed\n");
                return 1;
            }
            st->table = tmp;
            st->alloc_len = 2 * st->alloc_len;
            offset = st->use_len;
        }
    }
    st->table[offset].key = malloc(key_len);
    if (st->table[offset].key == NULL) {
        
    }
    st->table[offset].value = malloc(value_len);
    if (st->table[offset].value == NULL) {
        free(st->table[offset].key);
        return 1; // NG
    }
    memcpy(st->table[offset].key, key, key_len);
    memcpy(st->table[offset].value, value, value_len);
    st->table[offset].use = 1;
    if (offset == st->use_len) {
        st->use_len = offset + 1;
    }
}

char *
y_strtable_get(y_strtable_t *st, char *key, int key_len, int *value_len) {
    int i;
    for (i = 0 ; i < st->use_len ; i++) {
        if ((st->table[i].use) &&
            (st->table[i].key_len == key_len) &&
            (memcmp(st->table[i].key, key, key_len) == 0)) {
            *value_len = st->table[i].value_len;
            return st->table[i].key;
        }
    }
    return NULL;
}

int
y_strtable_delete(y_strtable_t *st, char *key, int key_len) {
    int i;
    for (i = 0 ; i < st->use_len ; i++) {
        if ((st->table[i].use) &&
            (st->table[i].key_len == key_len) &&
            (memcmp(st->table[i].key, key, key_len) == 0)) {
            free(st->table[i].key);
            free(st->table[i].value);
            st->table[i].key = 0;
            st->table[i].value = 0;
            st->table[i].use = 0;
            return 0;
        }
    }
    return 1;
}

void
y_strtable_close(y_strtable_t *st) {
    int i;
    for (i = 0 ; i < st->use_len ; i++) {
        if (st->table[i].use) {
            free(st->table[i].key);
            free(st->table[i].value);
        }
    }
    free(st->table);
    free(st);
}
