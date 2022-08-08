[BITS 64]

SECTION .text

global in1, out1, load_gdtr, load_tr, load_idtr
global enable_interrupt, disable_interrupt, read_rflags, read_tsc
global switch_context

in1:
    push rdx

    mov rdx, rdi
    mov rax, 0
    in al, dx
    
    pop rdx
    ret

out1:
    push rdx
    push rax

    mov rdx, rdi
    mov rax, rsi
    out dx, al

    pop rax
    pop rdx
    ret

load_gdtr:
    lgdt [ rdi ]
    ret

load_tr:
    ltr di
    ret

load_idtr:
    lidt [ rdi ]
    ret

enable_interrupt:
    sti
    ret

disable_interrupt:
    cli
    ret

read_rflags:
    pushfq
    pop rax
    ret

read_tsc:
    push rdx
    rdtsc
    shl rdx, 32
    or rax, rdx
    pop rdx
    ret

%macro SAVE_CONTEXT 0
    push rbp
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

switch_context:
    push rbp
    mov rbp, rsp

    pushfq
    cmp rdi, 0
    je .LOAD_CONTEXT
    popfq

    push rax
    mov ax, ss
    mov qword [ rdi + (23 * 8) ], rax ; save ss

    mov rax, rbp
    add rax, 16 ; exclude sfp, ret
    mov qword [ rdi + (22 * 8) ], rax ; save rsp

    pushfq
    pop rax
    mov qword [ rdi + (21 * 8) ], rax ; save RFLAGS

    mov ax, cs
    mov qword [ rdi + (20 * 8) ], rax ; save cs

    mov rax, qword [ rbp + 8 ]
    mov qword [ rdi + (19 * 8) ], rax ; save ret

    pop rax
    pop rbp

    add rdi, (19 * 8)
    mov rsp, rdi
    sub rdi, (19 * 8)

    SAVE_CONTEXT

    .LOAD_CONTEXT:
        mov rsp, rsi
        LOAD_CONTEXT
    
    iretq