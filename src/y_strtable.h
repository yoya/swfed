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
    //
    signed int get_offset;
} y_strtable_t;

// open/close
extern y_strtable_t *y_strtable_open();
extern void y_strtable_close(y_strtable_t *st);

// set/get/delete
extern int y_strtable_set(y_strtable_t *st, char *key, int key_len, char *value, int value_len);
extern char *y_strtable_get(y_strtable_t *st, char *key, int key_len,int *value_len);
extern int y_strtable_delete(y_strtable_t *st, char *key, int key_len);

// itelator
extern void y_strtable_rewind(y_strtable_t *st);
extern int y_strtable_hasnext(y_strtable_t *st);
extern char * y_strtable_get_currentkey(y_strtable_t *st, int *key_len);
extern char * y_strtable_get_currentvalue(y_strtable_t *st, int *value_len);


#endif /* __STRTABLE_H */
