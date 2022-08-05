#include "queue.h"
#include "utility.h"

void initialize_queue(struct generic_queue *queue, void *queue_buf,
                        unsigned int max_count, unsigned int data_size) {
    queue->max_count = max_count;
    queue->data_size = data_size;
    queue->buf = queue_buf;

    queue->put_idx = queue->get_idx = 0;
    queue->is_last_put = FALSE;
}

BOOL is_queue_full(struct generic_queue *queue) {
    if ((queue->get_idx == queue->put_idx) &&
        queue->is_last_put == TRUE) {
        return TRUE;
    }
    return FALSE;
}

BOOL is_queue_empty(struct generic_queue *queue) {
    if ((queue->get_idx == queue->put_idx) &&
        queue->is_last_put == FALSE) {
        return TRUE;
    }
    return FALSE;
}

BOOL put_queue(struct generic_queue *queue, void *data) {
    if (is_queue_full(queue)) {
        return FALSE;
    }
    memcpy((char *)queue->buf + (queue->data_size * queue->put_idx),
            data, 
            queue->data_size);
    queue->put_idx = (queue->put_idx + 1) % queue->max_count;
    queue->is_last_put = TRUE;

    return TRUE;
}

BOOL get_queue(struct generic_queue *queue, void *data) {
    if (is_queue_empty(queue)) {
        return FALSE;
    }
    memcpy(data,
            (char *)queue->buf + (queue->data_size *queue->get_idx), 
            queue->data_size);
    queue->get_idx = (queue->get_idx + 1) % queue->max_count;
    queue->is_last_put = FALSE;
    
    return TRUE;
}