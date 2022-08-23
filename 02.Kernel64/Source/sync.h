#ifndef __sync_h__
#define __sync_h__

#include "types.h"

#pragma pack(push, 1)

typedef struct _MUTEX {
    volatile QWORD task_id;
    volatile DWORD lock_count;

    volatile BOOL lock_flag;

    BYTE padding[3];
} MUTEX;

#pragma pack(pop)

BOOL lock_system(void);
void unlock_system(BOOL interrupt_flag);

void init_mutex(MUTEX *mutex);
void mutex_lock(MUTEX *mutex);
void mutex_unlock(MUTEX *mutex);

BOOL g_is_system_locked;
#endif