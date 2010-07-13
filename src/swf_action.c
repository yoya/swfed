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

int
swf_action_parse(bitstream_t *bs, swf_action_t *act) {
    unsigned long offset;
    bitstream_align(bs);
    act->action_id = bitstream_getbyte(bs);
    if (act->action_id & 0x80) {
        act->action_has_length = 1;
    } else {
        act->action_has_length = 0;
    }
    if (act->action_has_length) {
        act->action_length = bitstream_getbytesLE(bs, 2);
        offset = bitstream_getbytepos(bs);
        act->action_data = malloc(act->action_length);
        if (act->action_data == NULL) {
            fprintf(stderr, "Can't alloc memory for act->action_data\n");
            return 1;
        }
        bitstream_getstring(bs, act->action_data, act->action_length);
    }
    return 0;
}

int
swf_action_build(bitstream_t *bs, swf_action_t *act) {
    bitstream_align(bs);
    /* bitstream_putbits(bs, act->action_has_length, 1);  no need */
    bitstream_putbyte(bs, act->action_id);
    if (act->action_has_length) {
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
    if (act->action_has_length) {
        int i, n;
        unsigned char *d;
        switch(act->action_id) {
        case 0x83: // Get URL
            printf(" (String)%s", act->action_data);
            printf(" (String)%s",
                   act->action_data + strlen((char*) act->action_data) + 1);
            break;
        case 0x88: // Declare Dictionary
            d = act->action_data;
            n = GetUShortLE(act->action_data);  d += 2;
            printf(":\n");
            print_indent(indent_depth);
            for (i=0 ; i < n ; i++) {
                printf("\t\t[%d]'", i);
                d += printf("%s", d) + 1;
                printf("'\n");
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
swf_action_list_create(bitstream_t *bs) {
    swf_action_list_t *action_list;
    swf_action_t *action;
    action_list = calloc(sizeof(*action_list), 1);
    if (action_list == NULL) {
        fprintf(stderr, "Can't alloc memory for action_list\n");
        return NULL;
    }
    do {
        action = calloc(sizeof(*action), 1);
        if (action == NULL) {
            fprintf(stderr, "Can't alloc memory for action\n");
            break;
        }
        swf_action_parse(bs, action);
        if (action_list->head == NULL) {
            action_list->head = action_list->tail = action;
        } else {
            action_list->tail->next = action;
            action_list->tail = action;
        }
        action->next = NULL;
    } while(action->action_id != 0); // End Action;
    return action_list;
}
unsigned char *
swf_action_list_output(swf_action_list_t *list, unsigned long *length) {
    swf_action_t *action;
    bitstream_t *bs;
    unsigned char *data;
    *length = 0;
    bs = bitstream_open();
    for (action=list->head ; action ; action=action->next) {
        swf_action_build(bs, action);
    }
    data = bitstream_steal(bs, length);
    bitstream_close(bs);
    return data;
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

int swf_action_data_print(unsigned char *action_data, unsigned short action_data_len) {
    unsigned char type = action_data[0] & 0xff;
    unsigned char *data = action_data+1;
    unsigned short data_len = action_data_len - 1;
    int result = 1; // type field
    switch (type) {
    case 0x00: // String
        printf("(String)%*s", data_len, data);
        result += strlen((char*) data) + 1; // text + \0
        break;
    case 0x01: // Float
        printf("(Float)XXX");
        break;
    case 0x02: // NULL
        printf("(NULL)");
        break;
    case 0x03: // Undefined
        printf("(Undefined)");
        break;
    case 0x04: // Register
        printf("(Register)%d", (data[0]&0xff));
        break;
    case 0x05: // Boolean
        printf("(Boolean)%s", (data[0]&0xff)?"true":"false");
        result += 1;
        break;
    case 0x06: // Double
        printf("(Double)%f", GetDoubleIEEE(data));
        break;
    case 0x07: // Integer
        printf("(Integer)%ld", GetULongLE(data));
        result += 4;
        break;
    case 0x08: // Dictionary Lookup
        printf("(Dictionary Lookup)%d", data[0] & 0xff);
        result += 1;
        break;
    case 0x09: // Large Dictionary Lookup
        printf("(Large Dictionary Lookup)%d", GetUShortLE(data) & 0xffff);
        result += 2;
        break;
    default:
        printf("type=0x%02x len=%d", type, data_len);
        break;
    }
    return result;
}
