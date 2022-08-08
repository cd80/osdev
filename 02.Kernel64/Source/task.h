#ifndef __task_h__
#define __task_h__

#include "types.h"

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

#pragma pack(push, 1)

typedef struct _CONTEXT {
    QWORD registers[TASK_REGISTERCOUNT];
} CONTEXT;

typedef struct _TCB {
    CONTEXT ctx;

    QWORD id;
    QWORD flags;

    void *stack_address;
    QWORD stack_size;
} TCB;

#pragma pack(pop)

void setup_task(TCB *tcb, QWORD id, QWORD flags, QWORD entry_point,
                void *stack_address, QWORD stack_size);

#endif