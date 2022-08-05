#ifndef __queue_h__
#define __queue_h__

#include "types.h"

#pragma pack(push, 1)

struct generic_queue {
    unsigned int data_size;
    unsigned int max_count;

    void *buf;
    unsigned int put_idx;
    unsigned int get_idx;

    BOOL is_last_put;
};

#pragma pack(pop)

void initialize_queue(struct generic_queue *queue, void *queue_buf,
                        unsigned int max_count, unsigned int data_size);
BOOL is_queue_full(struct generic_queue *queue);
BOOL is_queue_empty(struct generic_queue *queue);
BOOL put_queue(struct generic_queue *queue, void *data);
BOOL get_queue(struct generic_queue *queue, void *data);

#endif