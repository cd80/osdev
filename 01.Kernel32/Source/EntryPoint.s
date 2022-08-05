[ORG 0x00]
[BITS 16]

SECTION .text

START:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax

    ; A20 gate
    mov ax, 0x2401
    int 0x15
    jc .A20BIOSERROR
    jmp .A20BIOSSUCCESS

    .A20BIOSERROR:
        in al, 0x92
        or al, 0x02
        and al, 0xfe
        out 0x92, al

    .A20BIOSSUCCESS:
    
    cli

    lgdt [ GDTR ]

    mov eax, 0x4000003B
    mov cr0, eax
    
    jmp dword 0x18 : ( PROTECTED_MODE - $$ + 0x10000 )
    nop
    nop


[BITS 32]
PROTECTED_MODE:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
    mov esp, 0xFFFC
    mov ebp, 0xFFFC

    push ( MSG_SWITCH_SUCCESS - $$ + 0x10000 )
    push 2
    push 0
    call PRINT_MESSAGE
    add esp, 12

    jmp 0x18 : 0x10200

PRINT_MESSAGE:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push eax
    push ecx
    push edx

    mov eax, dword [ ebp + 12 ]
    mov esi, 160
    mul esi
    mov edi, eax

    mov eax, dword [ ebp + 8 ]
    mov esi, 2
    mul esi
    add edi, eax

    mov esi, dword [ ebp + 16 ]

    .MESSAGE_LOOP:
        mov cl, byte [ esi ]
        cmp cl, 0
        je .MESSAGE_END

        mov byte [ 0xB8000 + edi ], cl

        add esi, 1
        add edi, 2
        jmp .MESSAGE_LOOP

    .MESSAGE_END:
        pop edx
        pop ecx
        pop eax
        pop edi
        pop esi
        pop ebp
    ret

align 8, db 0
dw 0x0000

GDTR:
    dw GDTEND - GDT - 1
    dd ( GDT - $$ + 0x10000 )

GDT:
    NULL_Descriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    x64_CODE_Descriptor:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xAF
        db 0x00

    x64_DATA_Descriptor:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xAF
        db 0x00

    x86_CODE_Descriptor:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00

    x86_DATA_Descriptor:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xCF
        db 0x00

GDTEND:

MSG_SWITCH_SUCCESS: db 'Switched to protected mode', 0

times 512 - ( $ - $$ ) db 0x00

