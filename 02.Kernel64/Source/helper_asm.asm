[BITS 64]

SECTION .text

global in1, out1, load_gdtr, load_tr, load_idtr
global enable_interrupt, disable_interrupt, read_rflags, read_tsc

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