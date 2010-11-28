#ifndef __STRTABLE_H
#define __STRTABLE_H

typedef struct _y_strtable_entry_t {
    char *key, *value;
    int key_len, value_len;
    int use;
} y_strtable_entry_t;

typedef struct _y_strtable_t {
    int use_len;
    int alloc_len;
    y_strtable_entry_t *table;
} y_strtable_t;

extern y_strtable_t *y_strtable_open();
extern int y_strtable_set(y_strtable_t *st, char *key, int key_len, char *value, int value_len);
extern char *y_strtable_get(y_strtable_t *st, char *key, int key_len,int *value_len);
extern int y_strtable_delete(y_strtable_t *st, char *key, int key_len);

extern void y_strtable_close(y_strtable_t *st);

#endif /* __STRTABLE_H */
