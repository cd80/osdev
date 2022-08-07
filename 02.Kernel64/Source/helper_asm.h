#ifndef __helper_asm_h__
#define __helper_asm_h__

#include "types.h"
BYTE in1(WORD port);
void out1(WORD port, BYTE data);
void load_gdtr(QWORD gdtr_address);
void load_tr(WORD tss_segment_offset);
void load_idtr(QWORD idtr_address);
void enable_interrupt(void);
void disable_interrupt(void);
QWORD read_rflags(void);
QWORD read_tsc(void);

#endif