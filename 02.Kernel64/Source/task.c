#include "task.h"
#include "descriptor.h"
#include "utility.h"
#include "helper_asm.h"
#include "console.h"
#include "sync.h"

static SCHEDULER scheduler;
static TCBPOOLMANAGER tcb_pool_manager;

static void initialize_tcb_pool(void) {
    memset(&(tcb_pool_manager), 0, sizeof(tcb_pool_manager));

    tcb_pool_manager.start_address = (TCB *)TASK_TCBPOOLADDRESS;
    memset(TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

    for (int i = 0; i < TASK_MAXCOUNT; ++i) {
        tcb_pool_manager.start_address[i].link.id = i;
    }

    tcb_pool_manager.max_count = TASK_MAXCOUNT;
    tcb_pool_manager.allocated_count = 1;
}

static TCB *allocate_tcb(void) {
    int i;
    TCB *empty_tcb;
    if (tcb_pool_manager.use_count == TASK_MAXCOUNT) {
        return NULL;
    }

    for (i = 0; i < tcb_pool_manager.max_count; ++i) {
        if ((tcb_pool_manager.start_address[i].link.id >> 32) == 0) {
            empty_tcb = &(tcb_pool_manager.start_address[i]);
            break;
        }
    }

    empty_tcb->link.id = ((QWORD)tcb_pool_manager.allocated_count << 32) | i;
    tcb_pool_manager.use_count++;
    tcb_pool_manager.allocated_count++;
    if (tcb_pool_manager.allocated_count == 0) {
        tcb_pool_manager.allocated_count = 1;
    }

    return empty_tcb;
}

static void free_tcb(QWORD id) {
    int i;
    i = GETTCBOFFSET(id);

    memset(&(tcb_pool_manager.start_address[i].ctx), 0, sizeof(CONTEXT));
    tcb_pool_manager.start_address[i].link.id = i;

    tcb_pool_manager.use_count--;
}

TCB *create_task(QWORD flags, void *memory_address,
                QWORD memory_size, QWORD entry_point) {
    TCB *task, *process;
    void *stack_address;
    BOOL prev_flag;

    prev_flag = lock_system();
    task = allocate_tcb();
    if (task == NULL) {
        unlock_system(prev_flag);
        return NULL;
    }

    process = get_process_by_thread(get_running_task());
    if (process == NULL) {
        free_tcb(task->link.id);
        unlock_system(prev_flag);
        return NULL;
    }

    if (flags & TASK_FLAGS_THREAD) {
        task->parent_pid = process->link.id;
        task->memory_address = process->memory_address;
        task->memory_size = process->memory_size;

        add_list_to_tail(&(process->child_thread_list), &(task->thread_link));
    }
    else {
        task->parent_pid = process->link.id;
        task->memory_address = memory_address;
        task->memory_size = memory_size;
    }

    task->thread_link.id = task->link.id;

    unlock_system(prev_flag);

    stack_address = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * 
                                            GETTCBOFFSET(task->link.id)));
    setup_task(task, flags, entry_point, stack_address, TASK_STACKSIZE);

    initialize_list(&(task->child_thread_list));

    prev_flag = lock_system();
    add_task_to_ready_list(task);
    unlock_system(prev_flag);

    return task;
}

static void setup_task(TCB *tcb, QWORD flags, QWORD entry_point,
                void *stack_address, QWORD stack_size) {
    memset(tcb->ctx.registers, 0, sizeof(tcb->ctx.registers));

    tcb->ctx.registers[TASK_RSPOFFSET] = (QWORD)stack_address + stack_size - 8;
    tcb->ctx.registers[TASK_RBPOFFSET] = (QWORD)stack_address + stack_size - 8;
    *(QWORD *)((QWORD)stack_address + stack_size - 8) = (QWORD)exit_task;

    tcb->ctx.registers[TASK_CSOFFSET] = GDT_KERNELCODESEGMENT;
    tcb->ctx.registers[TASK_DSOFFSET] = GDT_KERNELDATASEGMENT;
    tcb->ctx.registers[TASK_ESOFFSET] = GDT_KERNELDATASEGMENT;
    tcb->ctx.registers[TASK_FSOFFSET] = GDT_KERNELDATASEGMENT;
    tcb->ctx.registers[TASK_GSOFFSET] = GDT_KERNELDATASEGMENT;
    tcb->ctx.registers[TASK_SSOFFSET] = GDT_KERNELDATASEGMENT;

    tcb->ctx.registers[TASK_RIPOFFSET] = entry_point;
    tcb->ctx.registers[TASK_RFLAGSOFFSET] |= 0x0200;

    tcb->stack_address = stack_address;
    tcb->stack_size = stack_size;
    tcb->flags = flags;
}

BOOL initialize_scheduler(void) {
    TCB *task;
    initialize_tcb_pool();

    for (int i = 0; i < TASK_MAXREADYLISTCOUNT; ++i) {
        initialize_list(&(scheduler.ready_list[i]));
        scheduler.execute_count[i] = 0;
    }
    initialize_list(&(scheduler.wait_list));

    task = allocate_tcb();
    scheduler.running_task = task;
    task->flags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    task->parent_pid = task->link.id;
    task->memory_address = (void *)0x100000;
    task->memory_size = 0x500000;
    task->stack_address = (void *)0x600000;
    task->stack_size = 0x100000;

    scheduler.spent_processor_time_in_idle_task = 0;
    scheduler.processor_load = 0;

    return TRUE;
}

void set_running_task(TCB *task) {
    BOOL prev_flag;

    prev_flag = lock_system();
    scheduler.running_task = task;
    unlock_system(prev_flag);
}

TCB *get_running_task(void) {
    TCB *running_task;
    BOOL prev_flag;

    prev_flag = lock_system();
    running_task = scheduler.running_task;
    unlock_system(prev_flag);
    return running_task;
}

static TCB *get_next_task_to_run(void) {
    TCB *target = NULL;
    int task_count;
    for (int j = 0; j < 2; ++j) {
        for(int i = 0; i < TASK_MAXREADYLISTCOUNT; ++i) {
            task_count = get_list_count(&(scheduler.ready_list[i]));
            if (scheduler.execute_count[i] < task_count) {
                target = (TCB *)remove_list_from_header(&(scheduler.ready_list[i]));
                scheduler.execute_count[i]++;
                break;
            }
            else {
                scheduler.execute_count[i] = 0;
            }
        }
        if (target != NULL) {
            break;
        }
    }
    return target;
}

static BOOL add_task_to_ready_list(TCB *task) {
    BYTE priority;
    priority = GETPRIORITY(task->flags);
    if (priority >= TASK_MAXREADYLISTCOUNT) {
        SETPRIORITY(task->flags, TASK_FLAGS_MEDIUM);
        priority = TASK_FLAGS_MEDIUM;
    }
    add_list_to_tail(&(scheduler.ready_list[priority]), task);
    return TRUE;
}

static TCB *remove_task_from_ready_list(QWORD id) {
    TCB *target;
    BYTE priority;

    if (GETTCBOFFSET(id) >= TASK_MAXCOUNT) {
        return NULL;
    }

    target = &(tcb_pool_manager.start_address[GETTCBOFFSET(id)]);
    if (target->link.id != id) {
        return NULL;
    }

    priority = GETPRIORITY(target->flags);
    target = remove_list(&(scheduler.ready_list[priority]), id);
    return target;
}

BOOL change_priority(QWORD id, BYTE priority) {
    TCB *target;
    BOOL prev_flag;
    if (priority > TASK_MAXREADYLISTCOUNT) {
        return FALSE;
    }

    prev_flag = lock_system();

    target = scheduler.running_task;
    if (target->link.id == id) {
        SETPRIORITY(target->flags, priority);
    }
    else {
        target = remove_task_from_ready_list(id);
        if (target == NULL) {
            target = get_tcb_in_tcb_pool(GETTCBOFFSET(id));
            if (target != NULL) {
                SETPRIORITY(target->flags, priority);
            }
        }
        else {
            SETPRIORITY(target->flags, priority);
            add_task_to_ready_list(target);
        }
    }

    unlock_system(prev_flag);
    return TRUE;
}

void schedule(void) {
    TCB *running_task, *next_task;
    BOOL prev_flag;

    if (get_ready_task_count() < 1) {
        return;
    }

    prev_flag = lock_system();

    next_task = get_next_task_to_run();
    if (next_task == NULL) {
        unlock_system(prev_flag);
        return;
    }

    running_task = scheduler.running_task;
    scheduler.running_task = next_task;

    if ((running_task->flags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE) {
        scheduler.spent_processor_time_in_idle_task += TASK_PROCESSORTIME - scheduler.processor_time;
    }

    scheduler.processor_time = TASK_PROCESSORTIME;

    if (running_task->flags & TASK_FLAGS_ENDTASK) {
        add_list_to_tail(&(scheduler.wait_list), running_task);
        switch_context(NULL, &(next_task->ctx));
    }
    else {
        add_task_to_ready_list(running_task);
        switch_context(&(running_task->ctx), &(next_task->ctx));
    }

    unlock_system(prev_flag);
}

BOOL schedule_in_interrupt(void) {
    TCB *running_task, *next_task;
    char *ctx_address;
    BOOL prev_flag;

    prev_flag = lock_system();

    next_task = get_next_task_to_run();
    if (next_task == NULL) {
        unlock_system(prev_flag);
        return FALSE;
    }

    ctx_address = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    running_task = scheduler.running_task;
    scheduler.running_task = next_task;

    if ((running_task->flags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE) {
        scheduler.spent_processor_time_in_idle_task += TASK_PROCESSORTIME;
    }

    scheduler.processor_time = TASK_PROCESSORTIME;
    
    if (running_task->flags & TASK_FLAGS_ENDTASK) {
        add_list_to_tail(&(scheduler.wait_list), running_task);
    }
    else {
        memcpy(&(running_task->ctx), ctx_address, sizeof(CONTEXT));
        add_task_to_ready_list(running_task);
    }

    unlock_system(prev_flag);
    memcpy(ctx_address, &(next_task->ctx), sizeof(CONTEXT));
    
    return TRUE;
}

void decrease_processor_time(void) {
    if (scheduler.processor_time > 0) {
        scheduler.processor_time--;
    }
}

BOOL is_processor_time_expired(void) {
    if (scheduler.processor_time <= 0) {
        return TRUE;
    }
    return FALSE;
}

BOOL end_task(QWORD id) {
    TCB *target;
    BYTE priority;
    BOOL prev_flag;

    prev_flag = lock_system();

    target = scheduler.running_task;
    if (target->link.id == id) {
        target->flags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(target->flags, TASK_FLAGS_WAIT);

        unlock_system(prev_flag);
        schedule();
        while(1) {}
    }
    else {
        target = remove_task_from_ready_list(id);
        if (target == NULL) {
            target = get_tcb_in_tcb_pool(GETTCBOFFSET(id));
            if (target == NULL) {
                target->flags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY(target->flags, TASK_FLAGS_WAIT);
            }

            unlock_system(prev_flag);
            return TRUE;
        }

        target->flags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(target->flags, TASK_FLAGS_WAIT);
        add_list_to_tail(&(scheduler.wait_list), target);
    }

    unlock_system(prev_flag);
    return TRUE;
}

void exit_task(void) {
    end_task(scheduler.running_task->link.id);
}

int get_ready_task_count(void) {
    int total_count = 0;
    BOOL prev_flag;

    prev_flag = lock_system();
    for (int i = 0; i < TASK_MAXREADYLISTCOUNT; ++i) {
        total_count += get_list_count(&(scheduler.ready_list[i]));
    }
    unlock_system(prev_flag);
    return total_count;
}

int get_task_count(void) {
    int total_count;
    BOOL prev_flag;

    
    total_count = get_ready_task_count();

    prev_flag = lock_system();
    total_count += get_list_count(&(scheduler.wait_list)) + 1;
    unlock_system(prev_flag);
    return total_count;
}

TCB *get_tcb_in_tcb_pool(int offset) {
    if ((offset < 0) || (offset >= TASK_MAXCOUNT)) {
        return NULL;
    }

    return &(tcb_pool_manager.start_address[offset]);
}

BOOL is_task_exist(QWORD id) {
    TCB *tcb;
    tcb = get_tcb_in_tcb_pool(GETTCBOFFSET(id));
    if ((tcb == NULL) || (tcb->link.id != id)) {
        return FALSE;
    }
    return TRUE;
}

static TCB *get_process_by_thread(TCB *thread) {
    TCB *process;
    
    if (thread->flags & TASK_FLAGS_PROCESS) {
        return thread;
    }
    
    process = get_tcb_in_tcb_pool(GETTCBOFFSET(thread->parent_pid));

    if (((process == NULL) || (process->link.id != thread->parent_pid))) {
        return NULL;
    }

    return process;
}

QWORD get_processor_load(void) {
    return scheduler.processor_load;
}

void idle_task(void) {
    TCB *task, *child_thread, *process;;
    QWORD last_tick_count, last_spent_tick_in_idle_task;
    QWORD cur_tick_count, cur_spent_tick_in_idle_task;
    BOOL prev_flag;
    int count;
    void *thread_link;

    last_spent_tick_in_idle_task = scheduler.spent_processor_time_in_idle_task;
    last_tick_count = get_tick_count();

    while (1) {
        cur_tick_count = get_tick_count();
        cur_spent_tick_in_idle_task = scheduler.spent_processor_time_in_idle_task;

        if (cur_tick_count - last_tick_count == 0) {
            scheduler.processor_load = 0;
        }
        else {
            scheduler.processor_load = 100 - 
                    (cur_spent_tick_in_idle_task - last_spent_tick_in_idle_task) * 
                    100 / (cur_tick_count - last_tick_count);
        }

        last_tick_count = cur_tick_count;
        last_spent_tick_in_idle_task = cur_spent_tick_in_idle_task;

        halt_processor_by_load();

        if (get_list_count(&(scheduler.wait_list)) >= 0) {
            while (1) {
                prev_flag = lock_system();
                task = remove_list_from_header(&(scheduler.wait_list));
                if (task == NULL) {
                    unlock_system(prev_flag);
                    break;
                }

                if (task->flags & TASK_FLAGS_PROCESS) {
                    count = get_list_count(&(task->child_thread_list));
                    for (int i = 0; i < count; ++i) {
                        thread_link = (TCB *)remove_list_from_header(&(task->child_thread_list));
                        if (thread_link == NULL) {
                            break;
                        }

                        child_thread = GETTCBFROMTHREADLINK(thread_link);

                        add_list_to_tail(&(task->child_thread_list), &(child_thread->thread_link));

                        end_task(child_thread->link.id);
                    }

                    if (get_list_count(&(task->child_thread_list)) > 0) {
                        add_list_to_tail(&(scheduler.wait_list), task);
                        unlock_system(prev_flag);
                        continue;
                    }
                    else {
                        
                    }
                }
                else if (task->flags & TASK_FLAGS_THREAD) {
                    process = get_process_by_thread(task);
                    if (process != NULL) {
                        remove_list(&(process->child_thread_list), task->link.id);
                    }
                }
                printf("IDLE: Task ID[0x%q] is completely ended.\n", task->link.id);
                free_tcb(task->link.id);
                unlock_system(prev_flag);
            }
        }

        schedule();
    }
}

void halt_processor_by_load(void) {
    if (scheduler.processor_load < 40) {
        halt();
        halt();
        halt();
    }
    else if (scheduler.processor_load < 80) {
        halt();
        halt();
    }
    else if (scheduler.processor_load < 95) {
        halt();
    }
}