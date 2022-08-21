[BITS 32]
global read_cpuid, switch_to_64bit

SECTION .text

read_cpuid:
push ebp
mov ebp, esp
push eax
push ebx
push ecx
push edx
push edi

mov eax, dword [ ebp + 8 ]
cpuid

mov edi, dword [ ebp + 12 ]
mov dword [ edi ], eax
mov edi, dword [ ebp + 16 ]
mov dword [ edi ], ebx 
mov edi, dword [ ebp + 20 ]
mov dword [ edi ], ecx 
mov edi, dword [ ebp + 24 ]
mov dword [ edi ], edx 

pop edi
pop edx
pop ecx
pop ebx
pop eax
pop ebp
ret

switch_to_64bit:
    ; set PAE=1, OSXMMEXCPT=1, OSFXSR=1 in cr4
    mov eax, cr4
    or eax, 0x620
    mov cr4, eax

    ; set PML4 base in CR4
    mov eax, 0x100000
    mov cr3, eax

    ; set LME=1 in IA32_EFER to enable IA-32e
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x0100
    wrmsr

    ; set NW=0, CD=0, PG=1 to enable caching and paging
    ; set TS=1, EM=0, MP=1 to enable FPU
    mov eax, cr0
    or eax, 0xE000000E
    xor eax, 0x60000004
    mov cr0, eax

    jmp 0x08:0x200000

    jmp $
