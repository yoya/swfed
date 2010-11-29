/*
    gcc -W -Wall -D__KEYVALUE_DEBUG__ -DMALLOC_DEBUG y_keyvalue.c swf_debug.c
 */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "swf_define.h"
#include "y_keyvalue.h"

//#define Y_KEYVALUE_INITSIZE 10
#define Y_KEYVALUE_INITSIZE 1

/*
 * open/close
 */
y_keyvalue_t *
y_keyvalue_open() {
    y_keyvalue_t *st = calloc(sizeof(*st), 1);
    if (st == NULL) {
        fprintf(stderr, "y_keyvalue_open: calloc failed\n");
        return NULL;
    }
    st->use_len = 0;
    st->alloc_len = Y_KEYVALUE_INITSIZE;
    st->table = malloc(sizeof(*st->table) * Y_KEYVALUE_INITSIZE);
    return st;
}

void
y_keyvalue_close(y_keyvalue_t *st) {
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


/*
 * set/get/delete
 */

int
y_keyvalue_set(y_keyvalue_t *st, char *key, int key_len, char *value, int value_len) {
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
            tmp = realloc(st->table, 2 * st->alloc_len * sizeof(*(st->table)));
            if (tmp == NULL) {
                fprintf(stderr, "y_keyvalue_set: realloc failed\n");
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
    st->table[offset].key_len = key_len;
    st->table[offset].value_len = value_len;
    st->table[offset].use = 1;
    if (offset == st->use_len) {
        st->use_len = offset + 1;
    }
    return 0;
}

char *
y_keyvalue_get(y_keyvalue_t *st, char *key, int key_len, int *value_len) {
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
y_keyvalue_delete(y_keyvalue_t *st, char *key, int key_len) {
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

/*
 * itelator
 */
void
y_keyvalue_rewind(y_keyvalue_t *st) {
    st->get_offset = -1;
}
int
y_keyvalue_hasnext(y_keyvalue_t *st) {
    do {
        st->get_offset++;
        if (st->table[st->get_offset].use) {
            return 1; // found
        }
    } while (st->get_offset < st->use_len);

    return 0;// false
    
}
char *
y_keyvalue_get_currentkey(y_keyvalue_t *st, int *key_len) {
    if (st->get_offset >= st->use_len) {
        return NULL;
    }
    *key_len = st->table[st->get_offset].key_len;
    return st->table[st->get_offset].key;
}
char *
y_keyvalue_get_currentvalue(y_keyvalue_t *st, int *value_len) {
    if (st->get_offset >= st->use_len) {
        return NULL;
    }
    *value_len = st->table[st->get_offset].value_len;
    return st->table[st->get_offset].value;
}

#ifdef __KEYVALUE_DEBUG__

int main(void) {
    char *key, *value;
    int key_len, value_len;
    malloc_debug_start();
    y_keyvalue_t *st = y_keyvalue_open();
    y_keyvalue_set(st, "foo", 4, "baa", 4);
    y_keyvalue_set(st, "baz", 4, "buz", 4);
    y_keyvalue_rewind(st);
    while(y_keyvalue_hasnext(st)) {
        key = y_keyvalue_get_currentkey(st, &key_len);
        value = y_keyvalue_get_currentvalue(st, &value_len);
        printf("key=%s(%d), value=%s(%d)\n", key, key_len, value, value_len);
    }
    y_keyvalue_delete(st, "foo", 4);
    y_keyvalue_rewind(st);
    while(y_keyvalue_hasnext(st)) {
        key = y_keyvalue_get_currentkey(st, &key_len);
        value = y_keyvalue_get_currentvalue(st, &value_len);
        printf("key=%s(%d), value=%s(%d)\n", key, key_len, value, value_len);
    }
    y_keyvalue_close(st);
    malloc_debug_end();
    return 0;
}

#endif /* __KEYVALUE_DEBUG__ */
