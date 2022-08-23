#include "sync.h"
#include "helper_asm.h"
#include "task.h"
#include "utility.h"

BOOL lock_system(void) {
    return set_interrupt_flag(FALSE);
}

void unlock_system(BOOL interrupt_flag) {
    set_interrupt_flag(interrupt_flag);
}

void init_mutex(MUTEX *mutex) {
    mutex->lock_flag = FALSE;
    mutex->lock_count = 0;
    mutex->task_id = TASK_INVALIDID;
}

void mutex_lock(MUTEX *mutex) {
    if (test_and_set(&(mutex->lock_flag), 0, 1) == FALSE) {
        if (mutex->task_id == get_running_task()->link.id) {
            mutex->lock_count++;
            return;
        }
        
        while (test_and_set(&(mutex->lock_flag), 0, 1) == FALSE) {
            schedule();
        }
    }

    mutex->lock_count = 1;
    mutex->task_id = get_running_task()->link.id;
}

void mutex_unlock(MUTEX *mutex) {
    if ((mutex->lock_flag == FALSE) ||
        (mutex->task_id != get_running_task()->link.id)) {
        return;
    }

    if (mutex->lock_count > 1) {
        mutex->lock_count--;
    }
    
    mutex->task_id = TASK_INVALIDID;
    mutex->lock_count = 0;
    mutex->lock_flag = FALSE;
}