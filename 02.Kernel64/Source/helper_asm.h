#ifndef __helper_asm_h__
#define __helper_asm_h__

#include "types.h"
#include "task.h"

BYTE in1(WORD port);
WORD in2(WORD port);
void out1(WORD port, BYTE data);
void out2(WORD port, WORD data);
void load_gdtr(QWORD gdtr_address);
void load_tr(WORD tss_segment_offset);
void load_idtr(QWORD idtr_address);
void enable_interrupt(void);
void disable_interrupt(void);
QWORD read_rflags(void);
QWORD read_tsc(void);
void switch_context(CONTEXT *cur_ctx, CONTEXT *next_ctx);
void halt(void);
BOOL test_and_set(volatile BYTE *dest, BYTE cmp, BYTE src);

void init_fpu(void);
void save_fpu_ctx(void *fpu_ctx);
void load_fpu_ctx(void *fpu_ctx);
void set_ts(void);
void clear_ts(void);

#endif