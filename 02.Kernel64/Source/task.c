#include "task.h"
#include "descriptor.h"
#include "utility.h"
#include "helper_asm.h"
#include "console.h"

static SCHEDULER scheduler;
static TCBPOOLMANAGER tcb_pool_manager;

void initialize_tcb_pool(void) {
    memset(&(tcb_pool_manager), 0, sizeof(tcb_pool_manager));

    tcb_pool_manager.start_address = (TCB *)TASK_TCBPOOLADDRESS;
    memset(TASK_TCBPOOLADDRESS, 0, sizeof(TCB) * TASK_MAXCOUNT);

    for (int i = 0; i < TASK_MAXCOUNT; ++i) {
        tcb_pool_manager.start_address[i].link.id = i;
    }

    tcb_pool_manager.max_count = TASK_MAXCOUNT;
    tcb_pool_manager.allocated_count = 1;
}

TCB *allocate_tcb(void) {
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

void free_tcb(QWORD id) {
    int i;
    i = GETTCBOFFSET(id);

    memset(&(tcb_pool_manager.start_address[i].ctx), 0, sizeof(CONTEXT));
    tcb_pool_manager.start_address[i].link.id = i;

    tcb_pool_manager.use_count--;
}

TCB *create_task(QWORD flags, QWORD entry_point) {
    TCB *task;
    void *stack_address;

    task = allocate_tcb();
    if (task == NULL) {
        return NULL;
    }

    stack_address = (void *)(TASK_STACKPOOLADDRESS + (TASK_STACKSIZE * 
                                            GETTCBOFFSET(task->link.id)));
    setup_task(task, flags, entry_point, stack_address, TASK_STACKSIZE);
    add_task_to_ready_list(task);

    return task;
}

void setup_task(TCB *tcb, QWORD flags, QWORD entry_point,
                void *stack_address, QWORD stack_size) {
    memset(tcb->ctx.registers, 0, sizeof(tcb->ctx.registers));

    tcb->ctx.registers[TASK_RSPOFFSET] = (QWORD)stack_address + stack_size;
    tcb->ctx.registers[TASK_RBPOFFSET] = (QWORD)stack_address + stack_size;

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
    initialize_tcb_pool();

    for (int i = 0; i < TASK_MAXREADYLISTCOUNT; ++i) {
        initialize_list(&(scheduler.ready_list[i]));
        scheduler.execute_count[i] = 0;
    }
    initialize_list(&(scheduler.wait_list));

    scheduler.running_task = allocate_tcb();
    scheduler.running_task->flags = TASK_FLAGS_HIGHEST;

    scheduler.spent_processor_time_in_idle_task = 0;
    scheduler.processor_load = 0;

    return TRUE;
}

void set_running_task(TCB *task) {
    scheduler.running_task = task;
}

TCB *get_running_task(void) {
    return scheduler.running_task;
}

TCB *get_next_task_to_run(void) {
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

BOOL add_task_to_ready_list(TCB *task) {
    BYTE priority;
    priority = GETPRIORITY(task->flags);
    if (priority >= TASK_MAXREADYLISTCOUNT) {
        SETPRIORITY(task->flags, TASK_FLAGS_MEDIUM);
        priority = TASK_FLAGS_MEDIUM;
    }
    add_list_to_tail(&(scheduler.ready_list[priority]), task);
    return TRUE;
}

TCB *remove_task_from_ready_list(QWORD id) {
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
    if (priority > TASK_MAXREADYLISTCOUNT) {
        return FALSE;
    }

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

    return TRUE;
}

void schedule(void) {
    TCB *running_task, *next_task;
    BOOL prev_flag;

    if (get_ready_task_count() < 1) {
        return;
    }

    prev_flag = set_interrupt_flag(FALSE);

    next_task = get_next_task_to_run();
    if (next_task == NULL) {
        set_interrupt_flag(prev_flag);
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

    set_interrupt_flag(prev_flag);
}

BOOL schedule_in_interrupt(void) {
    TCB *running_task, *next_task;
    char *ctx_address;

    next_task = get_next_task_to_run();
    if (next_task == NULL) {
        return FALSE;
    }

    ctx_address = (char *)IST_STARTADDRESS + IST_SIZE - sizeof(CONTEXT);

    running_task = scheduler.running_task;
    scheduler.running_task = next_task;

    if ((running_task->flags & TASK_FLAGS_IDLE) == TASK_FLAGS_IDLE) {
        scheduler.spent_processor_time_in_idle_task += TASK_PROCESSORTIME;
    }
    
    if (running_task->flags & TASK_FLAGS_ENDTASK) {
        add_list_to_tail(&(scheduler.wait_list), running_task);
    }
    else {
        memcpy(&(running_task->ctx), ctx_address, sizeof(CONTEXT));
        add_task_to_ready_list(running_task);
    }
    memcpy(ctx_address, &(next_task->ctx), sizeof(CONTEXT));

    scheduler.processor_time = TASK_PROCESSORTIME;
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

    target = scheduler.running_task;
    if (target->link.id == id) {
        target->flags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(target->flags, TASK_FLAGS_WAIT);

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
            return FALSE; // ?
        }

        target->flags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY(target->flags, TASK_FLAGS_WAIT);
        add_list_to_tail(&(scheduler.wait_list), target);
    }
    return TRUE;
}

void exit_task(void) {
    end_task(scheduler.running_task->link.id);
}

int get_ready_task_count(void) {
    int total_count = 0;
    for (int i = 0; i < TASK_MAXREADYLISTCOUNT; ++i) {
        total_count += get_list_count(&(scheduler.ready_list[i]));
    }
    return total_count;
}

int get_task_count(void) {
    int total_count;
    
    total_count = get_ready_task_count();
    total_count += get_list_count(&(scheduler.wait_list)) + 1;

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

QWORD get_processor_load(void) {
    return scheduler.processor_load;
}

void idle_task(void) {
    TCB *task;
    QWORD last_tick_count, last_spent_tick_in_idle_task;
    QWORD cur_tick_count, cur_spent_tick_in_idle_task;

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
                task = remove_list_from_header(&(scheduler.wait_list));
                if (task == NULL) {
                    break;
                }
                printf("IDLE: Task ID[0x%q] is completely ended.\n", task->link.id);
                free_tcb(task->link.id);
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