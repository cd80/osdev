#include "task.h"
#include "descriptor.h"
#include "utility.h"
#include "helper_asm.h"

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
    i = id & 0xFFFFFFFF;

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
                                            (task->link.id & 0xFFFFFFFF)));
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
    initialize_list(&(scheduler.ready_list));
    scheduler.running_task = allocate_tcb();

    return TRUE;
}

void set_running_task(TCB *task) {
    scheduler.running_task = task;
}

TCB *get_running_task(void) {
    return scheduler.running_task;
}

TCB *get_next_task_to_run(void) {
    if (get_list_count(&(scheduler.ready_list)) == 0) {
        return NULL;
    }
    return (TCB *)remove_list_from_header(&(scheduler.ready_list));
}

void add_task_to_ready_list(TCB *task) {
    add_list_to_tail(&(scheduler.ready_list), task);
}

void schedule(void) {
    TCB *running_task, *next_task;
    BOOL prev_flag;

    if (get_list_count(&(scheduler.ready_list)) == 0) {
        return;
    }

    prev_flag = set_interrupt_flag(FALSE);

    next_task = get_next_task_to_run();
    if (next_task == NULL) {
        set_interrupt_flag(prev_flag);
        return;
    }

    running_task = scheduler.running_task;
    add_task_to_ready_list(running_task);
    
    scheduler.processor_time = TASK_PROCESSORTIME;

    scheduler.running_task = next_task;
    switch_context(&(running_task->ctx), &(next_task->ctx));

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
    memcpy(&(running_task->ctx), ctx_address, sizeof(CONTEXT));
    add_task_to_ready_list(running_task);

    scheduler.running_task = next_task;
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
