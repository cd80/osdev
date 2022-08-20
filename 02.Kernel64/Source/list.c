#include "list.h"

void initialize_list(LIST *list) {
    list->item_count = 0;
    list->header = NULL;
    list->tail = NULL;
}

int get_list_count(const LIST *list) {
    return list->item_count;
}

void add_list_to_tail(LIST *list, void *item) {
    LISTLINK *link;
    link = (LISTLINK *)item;
    link->next = NULL;
    if (list->header == NULL) {
        list->header = item;
        list->tail = item;
        list->item_count = 1;
        return;
    }

    link = (LISTLINK *)list->tail;
    link->next = item;

    list->tail = item;
    list->item_count++;
}

void add_list_to_header(LIST *list, void *item) {
    LISTLINK *link;
    link = (LISTLINK *)item;
    link->next = list->header;

    if (list->header == NULL) {
        list->header = item;
        list->tail = item;
        list->item_count = 1;
        return;
    }

    list->header = item;
    list->item_count++;
}

void *remove_list(LIST *list, QWORD id) {
    LISTLINK *link;
    LISTLINK *prev_link;

    prev_link = (LISTLINK *)list->header;
    for(link = prev_link; list != NULL; link = link->next) {
        if (link->id == id) {
            if (link == list->header && link == list->tail) {
                list->header = list->tail = NULL;
            }
            else if (link == list->header) {
                list->header = link->next;
            }
            else if (link == list->tail) {
                list->tail = prev_link;
            }
            else {
                prev_link->next = link->next;
            }

            list->item_count--;
            return link;
        }
        prev_link = link;
    }

    return NULL;
}

void *remove_list_from_header(LIST *list) {
    LISTLINK *link;
    if (list->item_count == 0) {
        return NULL;
    }
    link = (LISTLINK *)list->header;
    return remove_list(list, link->id);
}

void *remove_list_from_tail(LIST *list) {
    LISTLINK *link;
    if (list->item_count == 0) {
        return NULL;
    }

    link = (LISTLINK *)list->tail;
    return remove_list(list, link->id);
}

void *find_list(const LIST *list, QWORD id) {
    LISTLINK *link;
    for (link = (LISTLINK *)list->header; link != NULL; link = link->next) {
        if (link->id == id) {
            return link;
        }
    }
    return NULL;
}

void *get_header_from_list(const LIST *list) {
    return list->header;
}

void *get_tail_from_list(const LIST *list) {
    return list->tail;
}

void *get_next_from_list(const LIST *list, void *cur) {
    LISTLINK *link;
    link = (LISTLINK *)cur;
    return link->next;
}
