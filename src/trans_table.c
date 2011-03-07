#include <stdlib.h>
#include "trans_table.h"

trans_table_t *
trans_table_open() {
    trans_table_t *trans_table;
    trans_table = calloc(sizeof(*trans_table), 1);
    trans_table->table_num = 256; // XXX
    trans_table->table = calloc(trans_table->table_num, sizeof(int));
    return trans_table;
}
void
trans_table_close(trans_table_t *trans_table) {
    if (trans_table) {
        if (trans_table->table) {
            free(trans_table->table);
        }
        free(trans_table);
    }
}

int
trans_table_realloc(trans_table_t *trans_table, int offset) {
    int new_table_num = trans_table->table_num;
    int i;
    while (new_table_num <= offset) {
        new_table_num *= 2;
    }
    trans_table->table = realloc(trans_table->table, new_table_num);
    for (i = trans_table->table_num ; i < new_table_num ; i++) {
        trans_table->table[i] = 0;
    }
}
int
trans_table_get(trans_table_t  *trans_table, int offset) {
    if (trans_table->table_num <= offset) {
        return 0;
    }
    return trans_table->table[offset];
}
int
trans_table_set(trans_table_t  *trans_table, int offset, int cid) {
    int i, new_table_num;
    if (trans_table->table_num <= offset) {
        trans_table_realloc(trans_table, offset);
    }
    trans_table->table[offset] = cid;
}
int
trans_table_get_freeid(trans_table_t  *trans_table) {
    int i;
    // 0 は選択させない
    for (i = 1 ; i < trans_table->table_num ; i++) {
        if (trans_table->table[i] == 0) {
            return i;
        }
    }
    trans_table_realloc(trans_table, i);
    return i;
}
