#ifndef __list_h__
#define __list_h__

#include "types.h"

#pragma pack(push, 1)

typedef struct _LISTLINK {
    void *next;
    QWORD id;
} LISTLINK;

typedef struct _LIST {
    int item_count;
    void *header;
    void *tail;
} LIST;

#pragma pack(pop)

void initialize_list(LIST *list);
int get_list_count(const LIST *list);
void add_list_to_tail(LIST *list, void *item);
void add_list_to_header(LIST *list, void *item);
void *remove_list(LIST *list, QWORD id);
void *remove_list_from_header(LIST *list);
void *remove_list_from_tail(LIST *list);
void *find_list(const LIST *list, QWORD id);
void *get_header_from_list(const LIST *list);
void *get_tail_from_list(const LIST *list);
void *get_next_from_list(const LIST *list, void *cur);

#endif