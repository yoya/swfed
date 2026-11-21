#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "swf_action.h"

swf_action_info_t swf_action_info_table[] = {
    { 0x00, "End" },
    /* nothing 0x01 - 0x03 */
    { 0x04, "Next Frame" },
    { 0x05, "Previous Frame" },
    { 0x06, "Play" },
    { 0x07, "Stop" },
    { 0x08, "Toggle Quality" },
    { 0x09, "Stop Sound" },
    { 0x0A, "Add" },
    { 0x0B, "Subtract" },
    { 0x0C, "Multiply" },
    { 0x0D, "Divide" },
    { 0x0E, "Equal" },
    { 0x0F, "Less Than" },
    { 0x10, "Logical And" },
    { 0x11, "Logical Or " },
    { 0x12, "Logical Not" },
    { 0x13, "String Equal" },
    { 0x14, "String Length" },
    { 0x15, "SubString" },
    /* nothing 0x16 */
    { 0x17, "Pop"},
    { 0x18, "Integral Part"},
    /* nothing 0x19 - 0x1B */
    { 0x1C, "Get Variable"},
    { 0x1D, "Set Variable"},
    /* nothing 0x1E - 0x1F */
    { 0x20, "Set Target" },
    { 0x21, "Concatenate Strings"},
    { 0x22, "Get Property" },
    { 0x23, "Set Property" },
    { 0x24, "Duplicate Sprite" },
    { 0x25, "Remove Sprite" },
    { 0x26, "Trace" },
    { 0x27, "Start Drag" },
    { 0x28, "Stop Drag" },
    { 0x29, "String Less Than"},
    { 0x2A, "Throw" },
    { 0x2B, "Cast Object" },
    { 0x2C, "implements" },
    { 0x2D, "FSCommand2" },
    /* nothing 0x2E-2F */
    { 0x30, "Random" },
    { 0x31, "String Length(multibyte)" },
    { 0x32, "Ord" },
    { 0x33, "Chr"},
    { 0x34, "Get Timer" },
    { 0x35, "SubString(multibyte)" },
    { 0x36, "Ord(multibyte)" },
    { 0x37, "Chr(multibyte)" },
    /* nothing 0x28-29 */
    { 0x3A, "Delete" },
    { 0x3B, "Delete All" },
    { 0x3C, "Set Local Variable" },
    { 0x3D, "Call Function" },
    { 0x3E, "Return" },
    { 0x3F, "Modulo" },
    { 0x40, "New" },
    { 0x41, "Declare Local Variable" },
    { 0x42, "Declare Array" },
    { 0x43, "Declare Object" },
    { 0x44, "Type Of" },
    { 0x45, "Get Targer" },
    { 0x46, "Enumerate" },
    { 0x47, "Add(typed)" },
    { 0x48, "Less Than(typed)" },
    { 0x49, "Equal(typed)" },
    { 0x4A, "Number" },
    { 0x4B, "String" },
    { 0x4C, "Duplicate" },
    { 0x4D, "Swap" },
    { 0x4E, "Get Member" },
    { 0x4F, "Set Member" },
    { 0x50, "Increment" },
    { 0x51, "Decrement" },
    { 0x52, "Call Method" },
    { 0x53, "New Method" },
    { 0x54, "Instance Of" },
    { 0x55, "Enumerate Object" },
    /* nothing 0x56 - 0x5F */
    { 0x60, "And" },
    { 0x61, "Or" },
    { 0x62, "XOr" },
    { 0x63, "Shift Left" },
    { 0x64, "Shift Right" },
    { 0x65, "Shift Right Unsigned" },
    { 0x66, "Strict Equal" },
    { 0x67, "Greater Than(typed)" },
    { 0x68, "String Greater Than(typed)" },
    { 0x69, "Extends" },
    /* nothing 0x6A - 0x80 */
    { 0x81, "Goto Frame" },
    /* nothing 0x82 */
    { 0x83, "Get URL" },
    /* nothing 0x84 - 0x86 */
    { 0x87, "Store Register" },
    { 0x88, "Declare Dictionary" },
    { 0x89, "Strict Mode" },
    { 0x8A, "Wait For Frame" },
    { 0x8B, "Set Target" },
    { 0x8C, "Goto Label" },
    { 0x8D, "Wait For Frame(dynamic)" },
    { 0x8E, "Declare Function (with 256 registers)"},
    { 0x8F, "Try"},
    /* nothing 0x90 - 0x93 */
    { 0x94, "With"},
    /* nothing 0x95 */
    { 0x96, "Push Data" },
     /* nothing 0x97 - 0x98 */
    { 0x99, "Branch Always" },
    { 0x9A, "Get URL2" },
    { 0x9B, "Declare Function" },
    /* nothing 0x9C */
    { 0x9D, "Branch If True" },
    { 0x9E, "Call Frame" },
    { 0x9F, "Goto Expression" },
};

swf_action_info_t *
get_swf_action_info(int action_id) {
    int i, action_info_num = NumOfTable(swf_action_info_table);
    for (i=0 ; i < action_info_num; i++) {
        if (action_id == swf_action_info_table[i].id) {
            return &(swf_action_info_table[i]);
        }
    }
    return NULL;
}

/* length of NUL-terminated string in [p, end), or -1 if unterminated */
static long
_swf_action_strnlen(const unsigned char *p, const unsigned char *end) {
    const unsigned char *nul;
    if (p >= end) {
        return -1;
    }
    nul = memchr(p, '\0', end - p);
    if (nul == NULL) {
        return -1;
    }
    return nul - p;
}

int
swf_action_parse(bitstream_t *bs, swf_action_t *act) {
    int action_id;
    unsigned long action_length;
    bitstream_align(bs);
    action_id = bitstream_getbyte(bs);
    if (action_id < 0) {
        fprintf(stderr, "swf_action_parse: unexpected end of stream\n");
        return 1;
    }
    act->action_id = action_id;
    act->action_data = NULL;
    act->action_length = 0;
    if (act->action_id & 0x80) {
        action_length = bitstream_getbytesLE(bs, 2);
        if (action_length == (unsigned long) -1) {
            fprintf(stderr, "swf_action_parse: unexpected end of stream\n");
            return 1;
        }
        act->action_length = action_length;
        /* +1 for a guard NUL so string scanning never runs off the end */
        act->action_data = malloc(act->action_length + 1);
        if (act->action_data == NULL) {
            fprintf(stderr, "Can't alloc memory for act->action_data\n");
            return 1;
        }
        if (bitstream_getstring(bs, act->action_data, act->action_length)) {
            fprintf(stderr, "swf_action_parse: action_length(%d) exceeds stream\n",
                    act->action_length);
            free(act->action_data);
            act->action_data = NULL;
            return 1;
        }
        act->action_data[act->action_length] = '\0';
    }
    return 0;
}

int
swf_action_build(bitstream_t *bs, swf_action_t *act) {
    bitstream_align(bs);
    bitstream_putbyte(bs, act->action_id);
    if (act->action_id & 0x80) {
        if (act->action_data == NULL) {
            return 1; // error
        }
        bitstream_putbytesLE(bs, act->action_length, 2);
        bitstream_putstring(bs, act->action_data, act->action_length);
    }
    return 0;
}

int
swf_action_print(swf_action_t *act, int indent_depth) {
    swf_action_info_t *act_info = get_swf_action_info(act->action_id);
    if (act_info && act_info->name) {
        print_indent(indent_depth);
        printf("%s", act_info->name);
    } else {
        print_indent(indent_depth);
        printf("0x%02x", act->action_id);
    }
    if ((act->action_id & 0x80) && act->action_data) {
        int i, n;
        long len;
        unsigned char *d;
        unsigned char *end = act->action_data + act->action_length;
        switch(act->action_id) {
        case 0x83: // Get URL
            d = act->action_data;
            for (i = 0 ; i < 2 ; i++) {
                len = _swf_action_strnlen(d, end);
                if (len < 0) {
                    printf(" (broken)");
                    break;
                }
                printf(" (String)%s", d);
                d += len + 1;
            }
            break;
        case 0x88: // Declare Dictionary
            if (act->action_length < 2) {
                printf(" (broken)");
                break;
            }
            d = act->action_data;
            n = GetUShortLE(act->action_data);  d += 2;
            printf(":\n");
            print_indent(indent_depth);
            for (i=0 ; i < n ; i++) {
                len = _swf_action_strnlen(d, end);
                if (len < 0) {
                    printf("\t\t(broken)\n");
                    break;
                }
                printf("\t\t[%d]'%s'\n", i, d);
                d += len + 1;
            }
            break;
        case 0x96: // Push Data
            swf_action_data_print(act->action_data, act->action_length);
            break;
        default:
            printf(" len=%d", act->action_length);
            break;
        }
    }
    printf("\n");
    return 0;
}

swf_action_list_t *
swf_action_list_create(void) {
    swf_action_list_t *action_list;
    action_list = calloc(sizeof(*action_list), 1);
    if (action_list == NULL) {
        fprintf(stderr, "Can't alloc memory for action_list\n");
        return NULL;
    }
    action_list->head = NULL;
    action_list->tail = NULL;
    return action_list;
}

int
swf_action_list_parse(bitstream_t *bs, swf_action_list_t *action_list) {
    swf_action_t *action;
    while (bitstream_hasnext(bs, 1, 0)) {
        action = calloc(sizeof(*action), 1);
        if (action == NULL) {
            fprintf(stderr, "Can't alloc memory for action\n");
            break;
        }
        if (swf_action_parse(bs, action)) {
            fprintf(stderr, "swf_action_list_parse: swf_action_parse failed\n");
            free(action);
            return 1; // NG
        }
        if (action_list->head == NULL) {
            action_list->head = action_list->tail = action;
        } else {
            action_list->tail->next = action;
            action_list->tail = action;
        }
        action->next = NULL;
        if (action->action_id == 0) { // End Action;
            break;
        }
    }
    return 0;
}

int
swf_action_list_build(bitstream_t *bs, swf_action_list_t *list) {
    swf_action_t *action;
    for (action=list->head ; action ; action=action->next) {
        if (swf_action_build(bs, action) != 0) {
            fprintf(stderr, "swf_action_list_build: swf_action_build failed\n");
            bitstream_putbyte(bs, 0);
            return 1;
        }
    }
    return 0;
}

void
swf_action_list_destroy(swf_action_list_t *action_list) {

    if (action_list) {
        swf_action_t *action = action_list->head;
        while (action) {
            swf_action_t *action_next = action->next;
            if (action->action_data) {
                free(action->action_data);
            }
            free(action);
            action = action_next;
        }
        free(action_list);
    }
}

void
swf_action_list_print(swf_action_list_t *action_list, int indent_depth) {
    
    if (action_list) {
        swf_action_t *action = action_list->head;
        print_indent(indent_depth);
        printf("action list:\n");
        while(action) {
            swf_action_print(action, indent_depth);
            action = action->next;
        }
    }
}

int
swf_action_data_print(unsigned char *action_data, unsigned short action_data_len) {
    static const int action_value_type_size[] = {
        -1, 4, 0, 0, 1, 1, 8, 4, 1, 2
    };
    unsigned char type;
    unsigned char *data;
    unsigned char *end = action_data + action_data_len;
    unsigned short data_len = action_data_len - 1;
    long len;
    if (action_data == NULL) {
        return 1;
    }
    data = action_data;
    while (data < end) {
        type = data[0];
	data += 1;
        if ((type < 10) && (action_value_type_size[type] > 0) &&
            (end - data < action_value_type_size[type])) {
            printf("(broken)");
            break;
        }
	switch (type) {
          case 0x00: // String
            len = _swf_action_strnlen(data, end);
            if (len < 0) {
                printf("(broken)");
                return 1;
            }
	    printf("(String)%s", data);
	    data += len + 1; // text + \0
	    break;
          case 0x01: // Float
	    printf("(Float)%f", GetFloatIEEE(data));
	    data += 4;
	    break;
          case 0x02: // NULL
	    printf("(NULL)");
	    break;
          case 0x03: // Undefined
	    printf("(Undefined)");
	    break;
          case 0x04: // Register
	    printf("(Register)%d", (data[0]&0xff));
	    data += 1;
	    break;
          case 0x05: // Boolean
	    printf("(Boolean)%s", (data[0]&0xff)?"true":"false");
	    data += 1;
	    break;
          case 0x06: // Double
	    printf("(Double)%f", GetDoubleIEEE(data));
	    data += 8;
	    break;
          case 0x07: // Integer
	    printf("(Integer)%ld", GetULongLE(data));
	    data += 4;
	    break;
          case 0x08: // Dictionary Lookup
	    printf("(Dictionary Lookup)%d", data[0] & 0xff);
	    data += 1;
	    break;
          case 0x09: // Large Dictionary Lookup
	    printf("(Large Dictionary Lookup)%d", GetUShortLE(data) & 0xffff);
	    data += 2;
	    break;
          default:
	    printf("type=0x%02x len=%d", type, data_len);
	    break;
        }
    }
    return 0;
}

static int _swf_action_list_replace_strings(swf_action_list_t *action_list,
                                            int *modified, y_keyvalue_t *kv);

/* byte size of an action record */
static unsigned long
_swf_action_size(swf_action_t *action) {
    return (action->action_id < 0x80) ? 1 : 3 + (unsigned long) action->action_length;
}

/* start offsets of each action, plus the total length at [count] */
static long *
_swf_action_list_offsets(swf_action_list_t *action_list, int *count) {
    swf_action_t *action;
    long *pos;
    int n = 0, i = 0;
    for (action = action_list->head ; action ; action = action->next) {
        n++;
    }
    pos = malloc(sizeof(long) * (n + 1));
    if (pos == NULL) {
        return NULL;
    }
    pos[0] = 0;
    for (action = action_list->head ; action ; action = action->next) {
        pos[i + 1] = pos[i] + _swf_action_size(action);
        i++;
    }
    *count = n;
    return pos;
}

/* new offset of an old action boundary, or -1 */
static long
_swf_action_map_offset(long *old_pos, long *new_pos, int count, long offset) {
    int i;
    for (i = 0 ; i <= count ; i++) {
        if (old_pos[i] == offset) {
            return new_pos[i];
        }
    }
    return -1;
}

/* re-point a 16bit field that counts bytes from old_base to old_base + old_value */
static int
_swf_action_fix_field(unsigned char *field, long old_base, long new_base,
                      long old_value, long *old_pos, long *new_pos, int count,
                      int is_signed) {
    long target = _swf_action_map_offset(old_pos, new_pos, count, old_base + old_value);
    long value;
    if (target < 0) {
        return 1;
    }
    value = target - new_base;
    if (is_signed ? ((value < -32768) || (value > 32767)) : ((value < 0) || (value > 0xFFFF))) {
        return 1;
    }
    field[0] = value & 0xff;
    field[1] = (value >> 8) & 0xff;
    return 0;
}

/* After strings changed length, fix the fields that count bytes:
 * Jump / If BranchOffset, DefineFunction(2) codeSize, With Size, Try sizes. */
static int
_swf_action_list_fix_offsets(swf_action_list_t *action_list, long *old_pos, int count) {
    swf_action_t *action;
    long *new_pos;
    int n = 0, i = 0, ret = 0;
    new_pos = _swf_action_list_offsets(action_list, &n);
    if ((new_pos == NULL) || (n != count)) {
        free(new_pos);
        return 1;
    }
    for (action = action_list->head ; action && (ret == 0) ; action = action->next, i++) {
        unsigned char *d = action->action_data;
        unsigned short len = action->action_length;
        long old_end = old_pos[i + 1], new_end = new_pos[i + 1];
        if ((action->action_id < 0x80) || (d == NULL)) {
            continue;
        }
        switch (action->action_id) {
          case 0x99: // Jump
          case 0x9D: // If
            if (len >= 2) {
                long off = (short) (d[0] | (d[1] << 8));
                ret = _swf_action_fix_field(d, old_end, new_end, off,
                                            old_pos, new_pos, count, 1);
            }
            break;
          case 0x9B: // DefineFunction: codeSize is the last UI16
          case 0x8E: // DefineFunction2: codeSize is the last UI16
            if (len >= 2) {
                long size = d[len - 2] | (d[len - 1] << 8);
                ret = _swf_action_fix_field(d + len - 2, old_end, new_end, size,
                                            old_pos, new_pos, count, 0);
            }
            break;
          case 0x94: // With: Size
            if (len >= 2) {
                long size = d[0] | (d[1] << 8);
                ret = _swf_action_fix_field(d, old_end, new_end, size,
                                            old_pos, new_pos, count, 0);
            }
            break;
          case 0x8F: // Try: TrySize, CatchSize, FinallySize (blocks follow in order)
            if (len >= 7) {
                long try_size = d[1] | (d[2] << 8);
                long catch_size = d[3] | (d[4] << 8);
                long finally_size = d[5] | (d[6] << 8);
                long old_try_end = old_end + try_size;
                long old_catch_end = old_try_end + catch_size;
                long new_try_end = _swf_action_map_offset(old_pos, new_pos, count, old_try_end);
                long new_catch_end = _swf_action_map_offset(old_pos, new_pos, count, old_catch_end);
                if ((new_try_end < 0) || (new_catch_end < 0)) {
                    ret = 1;
                    break;
                }
                ret = _swf_action_fix_field(d + 1, old_end, new_end, try_size,
                                            old_pos, new_pos, count, 0) ||
                      _swf_action_fix_field(d + 3, old_try_end, new_try_end, catch_size,
                                            old_pos, new_pos, count, 0) ||
                      _swf_action_fix_field(d + 5, old_catch_end, new_catch_end, finally_size,
                                            old_pos, new_pos, count, 0);
            }
            break;
        }
    }
    free(new_pos);
    if (ret) {
        fprintf(stderr, "swf_action_list_replace_strings: can't fix branch offsets / sizes\n");
    }
    return ret;
}

/* a copy of the list (action_data has one more byte, as in swf_action_parse()) */
static swf_action_list_t *
_swf_action_list_clone(swf_action_list_t *action_list) {
    swf_action_list_t *clone = swf_action_list_create();
    swf_action_t *action;
    if (clone == NULL) {
        return NULL;
    }
    for (action = action_list->head ; action ; action = action->next) {
        swf_action_t *copy = calloc(sizeof(*copy), 1);
        if (copy == NULL) {
            swf_action_list_destroy(clone);
            return NULL;
        }
        *copy = *action;
        copy->action_data = NULL;
        copy->next = NULL;
        if (clone->head == NULL) { // linked first: destroy() then frees it too
            clone->head = copy;
        } else {
            clone->tail->next = copy;
        }
        clone->tail = copy;
        if (action->action_data) {
            copy->action_data = malloc((unsigned long) action->action_length + 1);
            if (copy->action_data == NULL) {
                swf_action_list_destroy(clone);
                return NULL;
            }
            memcpy(copy->action_data, action->action_data, action->action_length);
            copy->action_data[action->action_length] = '\0';
        }
    }
    return clone;
}

/*
 * The strings are replaced, and the branch offsets / sizes fixed, in a copy of
 * the list, which takes the place of the list only when all of it succeeded.
 * A failure half way leaves the list as it was: replaced strings with the
 * branch offsets of the old lengths must never be written out.
 */
int
swf_action_list_replace_strings(swf_action_list_t *action_list,
                                int *modified, y_keyvalue_t *kv) {
    int ret;
    long *old_pos;
    int count = 0, changed = 0;
    swf_action_list_t *work;
    if (modified) {
        *modified = 0;
    }
    work = _swf_action_list_clone(action_list);
    if (work == NULL) {
        return 1; // FAILURE
    }
    old_pos = _swf_action_list_offsets(work, &count);
    if (old_pos == NULL) {
        swf_action_list_destroy(work);
        return 1; // FAILURE
    }
    ret = _swf_action_list_replace_strings(work, &changed, kv);
    if ((ret == 0) && changed) {
        ret = _swf_action_list_fix_offsets(work, old_pos, count);
    }
    free(old_pos);
    if ((ret == 0) && changed) { // commit: swap the contents
        swf_action_t *head = action_list->head, *tail = action_list->tail;
        action_list->head = work->head;
        action_list->tail = work->tail;
        work->head = head;
        work->tail = tail;
        if (modified) {
            *modified = 1;
        }
    }
    swf_action_list_destroy(work);
    return ret;
}

static int
_swf_action_list_replace_strings(swf_action_list_t *action_list,
                                 int *modified, y_keyvalue_t *kv) {
    swf_action_t *action;
    *modified = 0;
    for (action=action_list->head ; action ; action=action->next) {
        int type;
        unsigned char *token, *end;
        long token_len;
        bitstream_t *bs;
        char *value;
        int value_len;
        int count;
        int m = 0;
        int i;
        if (action->action_id < 0x80) {
	    continue; // skip (no string)
	}
        if (action->action_data == NULL) {
            continue;
        }
        end = action->action_data + action->action_length;
	switch(action->action_id) {
	case 0x83: // Get URL
            bs = bitstream_open();
            token = action->action_data;
            for (i = 0 ; i < 2 ; i++) { // URL, target
                token_len = _swf_action_strnlen(token, end);
                if (token_len < 0) {
                    fprintf(stderr, "swf_action_list_replace_strings: broken GetURL string\n");
                    bitstream_close(bs);
                    return 1; // FAILURE
                }
                value = y_keyvalue_get(kv, (char *)token, token_len, &value_len);
                if (value) {
                    bitstream_putstring(bs, (unsigned char *) value, value_len);
                    m = 1;
                } else {
                    bitstream_putstring(bs, (unsigned char *) token, token_len);
                }
                bitstream_putbyte(bs, '\0');
                token += token_len + 1;
            }
	    break;
	case 0x88: // ActionConstantPool
            if (action->action_length < 2) {
                fprintf(stderr, "swf_action_list_replace_strings: broken ConstantPool\n");
                return 1; // FAILURE
            }
            count = action->action_data[0] + 0x100 * action->action_data[1];
            token = action->action_data + 2;
            bs = bitstream_open();
            bitstream_putbytesLE(bs, count, 2);
            for (i = 0 ; i < count ; i++) {
                token_len = _swf_action_strnlen(token, end);
                if (token_len < 0) {
                    fprintf(stderr, "swf_action_list_replace_strings: broken ConstantPool string (%d/%d)\n", i, count);
                    bitstream_close(bs);
                    return 1; // FAILURE
                }
                value = y_keyvalue_get(kv, (char *)token, token_len, &value_len);
                if (value) {
                    bitstream_putstring(bs, (unsigned char *)value, value_len);
                    m = 1;
                } else {
                    bitstream_putstring(bs, (unsigned char *)token , token_len);
                }
                bitstream_putbyte(bs, '\0');
                token += token_len + 1;
            }
	    break;
	case 0x96: // Push Data
            token = action->action_data;
            bs = bitstream_open();
	    while (token < end) {
	      static const int action_value_type_size[] = {
		-1, // 0: String
		4,  // 1: Float
		0,  // 2: NULL
		0,  // 3: Undefined
		1,  // 4: Register
		1,  // 5: Boolean
		8,  // 6: Double
		4,  // 7: Integer
		1,  // 8: Dictionary Lookup
		2,  // 9: Large Dictionary Lookup
	      };
	      type = token[0];
	      bitstream_putbyte(bs, type);
	      token += 1;
	      if (type == 0) { // String
                token_len = _swf_action_strnlen(token, end);
                if (token_len < 0) {
                    fprintf(stderr, "swf_action_list_replace_strings: broken Push string\n");
                    bitstream_close(bs);
                    return 1; // FAILURE
                }
                value = y_keyvalue_get(kv, (char *)token, token_len, &value_len);
                if (value) {
                    bitstream_putstring(bs, (unsigned char *)value, value_len);
                    m = 1;
                } else {
                    bitstream_putstring(bs, (unsigned char *)token , token_len);
                }
                bitstream_putbyte(bs, '\0');
                token += token_len + 1;
	      } else if (type < 10) { // else String
                if (end - token < action_value_type_size[type]) {
                    fprintf(stderr, "swf_action_list_replace_strings: broken Push value (type=%d)\n", type);
                    bitstream_close(bs);
                    return 1; // FAILURE
                }
		bitstream_putstring(bs, token, action_value_type_size[type]);
		token += action_value_type_size[type];
	      } else {
		fprintf(stderr, "swf_action_list_replace_strings: illegal type=%d\n", type);
		bitstream_close(bs);
		return 1; // FAILURE
	      }
	    }
	    break;
        default:
            continue;
	}
        /* note: token points into the old action_data, which is freed below */
        if (m) {
            unsigned long length;
            if (bitstream_length(bs) > 0xFFFF) {
                fprintf(stderr, "swf_action_list_replace_strings: action data too long (%lu)\n",
                        bitstream_length(bs));
                bitstream_close(bs);
                return 1; // FAILURE
            }
            free(action->action_data);
            action->action_data = bitstream_steal(bs, &length);
            action->action_length = length;
            if (modified) {
                *modified = 1;
            }
        }
        bitstream_close(bs);
    }
    return 0;
}

int
swf_action_list_append_top(swf_action_list_t *list, int action_id,
                           unsigned char *action_data, int action_data_length) {
    swf_action_t *action;
    action = calloc(sizeof(*action), 1);
    if (action == NULL) {
        return 1;
    }
    action->action_id = action_id;
    if (action_id >= 0x80) {
        if ((action_data_length < 0) || (action_data_length > 0xFFFF)) {
            free(action);
            return 1;
        }
        action->action_data = malloc(action_data_length + 1);
        if (action->action_data == NULL) {
            free(action);
            return 1;
        }
        memcpy(action->action_data, action_data, action_data_length);
        action->action_data[action_data_length] = '\0';
        action->action_length = action_data_length;
    } else {
        action->action_data = NULL;
        action->action_length = action_data_length;
    }
    action->next = list->head;
    list->head = action;
    if (list->tail == NULL) {
        list->tail = action;
    }
    return 0;
}

