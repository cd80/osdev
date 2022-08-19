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

#define TASK_STACKPOOLADDRESS   (TASK_TCBPOOLADDRESS + (sizeof(TCB) * TASK_MAXCOUNT))
#define TASK_STACKSIZE      8192
#define TASK_INVALIDID      0xFFFFFFFFFFFFFFFF

#define TASK_PROCESSORTIME  5

#define TASK_MAXREADYLISTCOUNT  5
#define TASK_FLAGS_HIGHEST  0
#define TASK_FLAGS_HIGH     1
#define TASK_FLAGS_MEDIUM   2
#define TASK_FLAGS_LOW      3
#define TASK_FLAGS_LOWEST   4
#define TASK_FLAGS_WAIT     0xFF

#define TASK_FLAGS_ENDTASK  0x8000000000000000
#define TASK_FLAGS_IDLE     0x0800000000000000

#define GETPRIORITY(x)      ((x) & 0xFF)
#define SETPRIORITY(x, p)   ((x) = ((x) & 0xFFFFFFFFFFFFFF00) | (p))
#define GETTCBOFFSET(x)     ((x) & 0xFFFFFFFF)     

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
    LIST ready_list[TASK_MAXREADYLISTCOUNT];
    LIST wait_list;
    int execute_count[TASK_MAXREADYLISTCOUNT];
    QWORD processor_load;
    QWORD spent_processor_time_in_idle_task;
} SCHEDULER;

#pragma pack(pop)

static void initialize_tcb_pool(void);
static TCB *allocate_tcb(void);
static void free_tcb(QWORD id);
TCB *create_task(QWORD flags, QWORD entry_point);
static void setup_task(TCB *tcb, QWORD flags, QWORD entry_point,
                void *stack_address, QWORD stack_size);

BOOL initialize_scheduler(void);
void set_running_task(TCB *task);
TCB *get_running_task(void);
static TCB *get_next_task_to_run(void);
static BOOL add_task_to_ready_list(TCB *task);
void schedule(void);
BOOL schedule_in_interrupt(void);
void decrease_processor_time(void);
BOOL is_processor_time_expired(void);
static TCB *remove_task_from_ready_list(QWORD id);
BOOL change_priority(QWORD id, BYTE priority);
BOOL end_task(QWORD id);
void exit_task(void);
int get_ready_task_count(void);
int get_task_count(void);
TCB *get_tcb_in_tcb_pool(int offset);
BOOL is_task_exist(QWORD id);
QWORD get_processor_load(void);

void idle_task(void);
void halt_processor_by_load(void);

#endif