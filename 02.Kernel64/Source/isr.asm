[BITS 64]

SECTION .text

extern common_exception_handler, common_interrupt_handler, keyboard_handler
extern timer_handler, device_not_available_handler, hdd_handler

; ISR for Exception
global isr_divide_error, isr_debug, isr_nmi, isr_breakpoint, isr_overflow
global isr_bound_range_exceeded, isr_invalid_opcode, isr_device_not_available
global isr_double_fault, isr_coprocessor_segment_overrun, isr_invalid_tss
global isr_segment_not_present, isr_stack_segfault, isr_general_protection
global isr_page_fault, isr_15, isr_fpu_error, isr_alignment_check
global isr_machine_check, isr_simd_error, isr_etc_exception

; ISR for Interrupt
global isr_timer, isr_keyboard, isr_slave_pic, isr_serial2, isr_serial1
global isr_parallel2, isr_floppy, isr_parallel1, isr_rtc, isr_reserved
global isr_not_used1, isr_not_used2, isr_mouse, isr_coprocessor
global isr_hdd1, isr_hdd2, isr_etc_interrupt

%macro SAVE_CONTEXT 0
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov ax, ds
    push rax
    mov ax, es
    push rax
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
%endmacro

%macro LOAD_CONTEXT 0
    pop gs
    pop fs
    pop rax
    mov es, ax
    pop rax
    mov ds, ax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
%endmacro

; ############ print exception handlers #############
; decl = """global isr_divide_error, isr_debug, isr_nmi, isr_breakpoint, isr_overflow
; global isr_bound_range_exceeded, isr_invalid_opcode, isr_device_not_available
; global isr_double_fault, isr_coprocessor_segment_overrun, isr_invalid_tss
; global isr_segment_not_present, isr_stack_segfault, isr_general_protection
; global isr_page_fault, isr_15, isr_fpu_error, isr_alignment_check
; global isr_machine_check, isr_simd_error, isr_etc_exception""".replace("global ", "").replace(" ", "").replace("\n", ",")
; decl = decl.split(",")
; exception_errorcode_map = [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0]
; temp_no_errorcode = """; Exception #{} {}
; {}:
;     SAVE_CONTEXT

;     mov rdi, {}
;     call common_exception_handler

;     LOAD_CONTEXT
;     iretq
; """

; temp_with_errorcode = """; Exception #{} {}
; {}:
;     SAVE_CONTEXT
    
;     mov rdi, {}
;     mov rsi, qword [ rbp + 8 ]
;     call common_exception_handler
    
;     LOAD_CONTEXT
;     add rsp, 8 ; remove error code from the stack
;     iretq
; """

; for i in range(0, len(decl)):
;     comment = decl[i].replace("isr_", "")
;     comment = ' '.join([x[0].upper() + x[1:] for x in comment.split("_")])
;     if exception_errorcode_map[i] == 0:
;         print(temp_no_errorcode.format(i, comment, decl[i], i))
;     else:
;         print(temp_with_errorcode.format(i, comment, decl[i], i))

; ######### print interrupt handlers ##########        

; decl = """global isr_timer, isr_keyboard, isr_slave_pic, isr_serial2, isr_serial1
; global isr_parallel2, isr_floppy, isr_parallel1, isr_rtc, isr_reserved
; global isr_not_used1, isr_not_used2, isr_mouse, isr_coprocessor
; global isr_hdd1, isr_hdd2, isr_etc_interrupt""".replace("global ", "").replace(" ", "").replace("\n", ",")
; decl = decl.split(",")
; handler = ["common_interrupt_handler" for i in range(0, len(decl))]
; handler[1] = "keyboard_handler"
; temp = """; Interrupt #{} {}
; {}:
;     SAVE_CONTEXT

;     mov rdi, {}
;     call {}

;     LOAD_CONTEXT
;     iretq
; """


; for i in range(0, len(decl)):
;     comment = decl[i].replace("isr_", "")
;     comment = ' '.join([x[0].upper() + x[1:] for x in comment.split("_")])
;     print(temp.format(i+32, comment, decl[i], i+32, handler[i]))

; Exception #0 Divide Error
isr_divide_error:
    SAVE_CONTEXT

    mov rdi, 0
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #1 Debug
isr_debug:
    SAVE_CONTEXT

    mov rdi, 1
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #2 Nmi
isr_nmi:
    SAVE_CONTEXT

    mov rdi, 2
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #3 Breakpoint
isr_breakpoint:
    SAVE_CONTEXT

    mov rdi, 3
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #4 Overflow
isr_overflow:
    SAVE_CONTEXT

    mov rdi, 4
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #5 Bound Range Exceeded
isr_bound_range_exceeded:
    SAVE_CONTEXT

    mov rdi, 5
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #6 Invalid Opcode
isr_invalid_opcode:
    SAVE_CONTEXT

    mov rdi, 6
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #7 Device Not Available
isr_device_not_available:
    SAVE_CONTEXT

    mov rdi, 7
    call device_not_available_handler

    LOAD_CONTEXT
    iretq

; Exception #8 Double Fault
isr_double_fault:
    SAVE_CONTEXT
    
    mov rdi, 8
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #9 Coprocessor Segment Overrun
isr_coprocessor_segment_overrun:
    SAVE_CONTEXT

    mov rdi, 9
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #10 Invalid Tss
isr_invalid_tss:
    SAVE_CONTEXT
    
    mov rdi, 10
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #11 Segment Not Present
isr_segment_not_present:
    SAVE_CONTEXT
    
    mov rdi, 11
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #12 Stack Segfault
isr_stack_segfault:
    SAVE_CONTEXT
    
    mov rdi, 12
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #13 General Protection
isr_general_protection:
    SAVE_CONTEXT
    
    mov rdi, 13
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #14 Page Fault
isr_page_fault:
    SAVE_CONTEXT
    
    mov rdi, 14
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #15 15
isr_15:
    SAVE_CONTEXT

    mov rdi, 15
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #16 Fpu Error
isr_fpu_error:
    SAVE_CONTEXT

    mov rdi, 16
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #17 Alignment Check
isr_alignment_check:
    SAVE_CONTEXT
    
    mov rdi, 17
    mov rsi, qword [ rbp + 8 ]
    call common_exception_handler
    
    LOAD_CONTEXT
    add rsp, 8 ; remove error code from the stack
    iretq

; Exception #18 Machine Check
isr_machine_check:
    SAVE_CONTEXT

    mov rdi, 18
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #19 Simd Error
isr_simd_error:
    SAVE_CONTEXT

    mov rdi, 19
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Exception #20 Etc Exception
isr_etc_exception:
    SAVE_CONTEXT

    mov rdi, 20
    call common_exception_handler

    LOAD_CONTEXT
    iretq

; Interrupt #32 Timer
isr_timer:
    SAVE_CONTEXT

    mov rdi, 32
    call timer_handler

    LOAD_CONTEXT
    iretq

; Interrupt #33 Keyboard
isr_keyboard:
    SAVE_CONTEXT

    mov rdi, 33
    call keyboard_handler

    LOAD_CONTEXT
    iretq

; Interrupt #34 Slave Pic
isr_slave_pic:
    SAVE_CONTEXT

    mov rdi, 34
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #35 Serial2
isr_serial2:
    SAVE_CONTEXT

    mov rdi, 35
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #36 Serial1
isr_serial1:
    SAVE_CONTEXT

    mov rdi, 36
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #37 Parallel2
isr_parallel2:
    SAVE_CONTEXT

    mov rdi, 37
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #38 Floppy
isr_floppy:
    SAVE_CONTEXT

    mov rdi, 38
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #39 Parallel1
isr_parallel1:
    SAVE_CONTEXT

    mov rdi, 39
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #40 Rtc
isr_rtc:
    SAVE_CONTEXT

    mov rdi, 40
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #41 Reserved
isr_reserved:
    SAVE_CONTEXT

    mov rdi, 41
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #42 Not Used1
isr_not_used1:
    SAVE_CONTEXT

    mov rdi, 42
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #43 Not Used2
isr_not_used2:
    SAVE_CONTEXT

    mov rdi, 43
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #44 Mouse
isr_mouse:
    SAVE_CONTEXT

    mov rdi, 44
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #45 Coprocessor
isr_coprocessor:
    SAVE_CONTEXT

    mov rdi, 45
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq

; Interrupt #46 Hdd1
isr_hdd1:
    SAVE_CONTEXT

    mov rdi, 46
    call hdd_handler

    LOAD_CONTEXT
    iretq

; Interrupt #47 Hdd2
isr_hdd2:
    SAVE_CONTEXT

    mov rdi, 47
    call hdd_handler

    LOAD_CONTEXT
    iretq

; Interrupt #48 Etc Interrupt
isr_etc_interrupt:
    SAVE_CONTEXT

    mov rdi, 48
    call common_interrupt_handler

    LOAD_CONTEXT
    iretq