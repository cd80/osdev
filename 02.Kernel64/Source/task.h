#ifndef __task_h__
#define __task_h__

#include "types.h"
#include "list.h"

#define TASK_REGISTERCOUNT  (5 + 19)
#define TASK_REGISTERSIZE   8

#define TASK_GSOFFSET   0
#define TASK_FSOFFSET   1
#define TASK_ESOFFSET   2
#define TASK_DSOFFSET   3
#define TASK_R15OFFSET  4
#define TASK_R14OFFSET  5
#define TASK_R13OFFSET  6
#define TASK_R12OFFSET  7
#define TASK_R11OFFSET  8
#define TASK_R10OFFSET  9
#define TASK_R9OFFSET   10
#define TASK_R8OFFSET   11
#define TASK_RSIOFFSET  12
#define TASK_RDIOFFSET  13
#define TASK_RDXOFFSET  14
#define TASK_RCXOFFSET  15
#define TASK_RBXOFFSET  16
#define TASK_RAXOFFSET  17
#define TASK_RBPOFFSET  18
#define TASK_RIPOFFSET  19
#define TASK_CSOFFSET   20
#define TASK_RFLAGSOFFSET   21
#define TASK_RSPOFFSET  22
#define TASK_SSOFFSET   23

#define TASK_TCBPOOLADDRESS ((void *)(0x800000))
#define TASK_MAXCOUNT       1024

#define TASK_STACKPOOLADDRESS   (TASK_TCBPOOLADDRESS + sizeof(TCB) * TASK_MAXCOUNT)
#define TASK_STACKSIZE      8192
#define TASK_INVALIDID      0xFFFFFFFFFFFFFFFF

#define TASK_PROCESSORTIME  5

#pragma pack(push, 1)

typedef struct _CONTEXT {
    QWORD registers[TASK_REGISTERCOUNT];
} CONTEXT;

typedef struct _TCB {
    LISTLINK link;

    CONTEXT ctx;

    QWORD id;
    QWORD flags;

    void *stack_address;
    QWORD stack_size;
} TCB;

typedef struct _TCBPOOLMANAGER {
    TCB *start_address;
    int max_count;
    int use_count;

    int allocated_count;
} TCBPOOLMANAGER;

typedef struct _SCHEDULER {
    TCB *running_task;
    int processor_time;
    LIST ready_list;
} SCHEDULER;

#pragma pack(pop)

void initialize_tcb_pool(void);
TCB *allocate_tcb(void);
void free_tcb(QWORD id);
TCB *create_task(QWORD flags, QWORD entry_point);
void setup_task(TCB *tcb, QWORD flags, QWORD entry_point,
                void *stack_address, QWORD stack_size);

BOOL initialize_scheduler(void);
void set_running_task(TCB *task);
TCB *get_running_task(void);
TCB *get_next_task_to_run(void);
void add_task_to_ready_list(TCB *task);
void schedule(void);
BOOL schedule_in_interrupt(void);
void decrease_processor_time(void);
BOOL is_processor_time_expired(void);

#endif