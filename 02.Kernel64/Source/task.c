#include "task.h"
#include "descriptor.h"
#include "utility.h"

void setup_task(TCB *tcb, QWORD id, QWORD flags, QWORD entry_point,
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

    tcb->id = id;
    tcb->stack_address = stack_address;
    tcb->stack_size = stack_size;
    tcb->flags = flags;
}