#ifndef __isr_h__
#define __isr_h__

void isr_divide_error(void);
void isr_debug(void);
void isr_nmi(void);
void isr_breakpoint(void);
void isr_overflow(void);
void isr_bound_range_exceeded(void);
void isr_invalid_opcode(void);
void isr_device_not_available(void);
void isr_double_fault(void);
void isr_coprocessor_segment_overrun(void);
void isr_invalid_tss(void);
void isr_segment_not_present(void);
void isr_stack_segfault(void);
void isr_general_protection(void);
void isr_page_fault(void);
void isr_15(void);
void isr_fpu_error(void);
void isr_alignment_check(void);
void isr_machine_check(void);
void isr_simd_error(void);
void isr_etc_exception(void);

void isr_timer(void);
void isr_keyboard(void);
void isr_slave_pic(void);
void isr_serial2(void);
void isr_serial1(void);
void isr_parallel2(void);
void isr_floppy(void);
void isr_parallel1(void);
void isr_rtc(void);
void isr_reserved(void);
void isr_not_used1(void);
void isr_not_used2(void);
void isr_mouse(void);
void isr_coprocessor(void);
void isr_hdd1(void);
void isr_hdd2(void);
void isr_etc_interrupt(void);

void *EXCEPTION_HANDLERS[] = {isr_divide_error,isr_debug,isr_nmi,isr_breakpoint,
    isr_overflow,isr_bound_range_exceeded,isr_invalid_opcode,
    isr_device_not_available,isr_double_fault,
    isr_coprocessor_segment_overrun,isr_invalid_tss,isr_segment_not_present,
    isr_stack_segfault,isr_general_protection,isr_page_fault,isr_15,
    isr_fpu_error,isr_alignment_check,isr_machine_check,isr_simd_error,
    isr_etc_exception};

void *INTERRUPT_HANDLERS[] = {
    isr_timer,isr_keyboard,isr_slave_pic,isr_serial2,
    isr_serial1,isr_parallel2,isr_floppy,isr_parallel1,isr_rtc,isr_reserved,
    isr_not_used1,isr_not_used2,isr_mouse,isr_coprocessor,isr_hdd1,
    isr_hdd2,isr_etc_interrupt};
#endif