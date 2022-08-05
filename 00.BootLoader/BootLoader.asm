[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07c0:START

TOTALSECTORCOUNT: dw 2
KERNEL32SECTORCOUNT: dw 2
START:
	mov ax, 0x07c0
	mov ds, ax
	mov ax, 0xB800
	mov es, ax

	mov ax, 0x0000
	mov ss, ax
	mov sp, 0xfffe
	mov bp, 0xfffe

	mov di, 0
	.CLEAN_VIDEOMEM:
		mov byte [ es : di ], 0
		mov byte [ es : di + 1 ], 0x07
		
		add di, 2
		cmp di, 80 * 25 * 2
		jl .CLEAN_VIDEOMEM
	
	push MSG_FIRST
	push 0
	push 0
	call PRINT_MESSAGE
	add sp, 6

	push MSG_IMAGE_LOADING
	push 1
	push 0
	call PRINT_MESSAGE
	add sp, 6
	
	.RESET_DISK:
		mov ax, 0
		mov dl, 0
		int 0x13
		jc .HANDLE_DISK_ERROR

		mov si, 0x1000
		mov es, si
		mov bx, 0x0000

		mov di, word [ TOTALSECTORCOUNT ]
	
	.READ_DATA:
		cmp di, 0
		je .READ_END
		sub di, 0x1

		mov ah, 0x02
		mov al, 0x1
		mov ch, byte [ TRACK_NUMBER ]
		mov cl, byte [ SECTOR_NUMBER ]
		mov dh, byte [ HEAD_NUMBER ]
		mov dl, 0x00
		int 0x13
		
		jc .HANDLE_DISK_ERROR
	
		add si, 0x0020
		mov es, si

		mov al, byte [ SECTOR_NUMBER ]
		add al, 0x01
		mov byte [ SECTOR_NUMBER ], al
		cmp al, 37
		jl .READ_DATA

		xor byte [ HEAD_NUMBER ], 0x01
		mov byte [ SECTOR_NUMBER ], 0x01

		cmp byte [ HEAD_NUMBER ], 0x00
		jne .READ_DATA
		
		add byte [ TRACK_NUMBER ], 0x01
		jmp .READ_DATA
	
	.READ_END:

		push MSG_IMAGE_LOADED
		push 1
		push 20
		call PRINT_MESSAGE
		add sp, 6

		jmp 0x1000:0000
	
	.HANDLE_DISK_ERROR:
		push MSG_DISK_ERROR
		push 1
		push 20
		call PRINT_MESSAGE

		jmp $
	
PRINT_MESSAGE:
	push bp
	mov bp, sp
	
	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800
	mov es, ax

	mov ax, word [ bp + 6 ]
	mov si, 160
	mul si
	mov di, ax

	mov ax, word [ bp + 4]
	mov si, 2
	mul si
	add di, ax

	mov si, word [ bp + 8 ]

	.MESSAGE_LOOP:
		mov cl, byte [ si ]
		cmp cl, 0
		je .MESSAGE_END

		mov byte [ es : di ], cl
		add si, 1
		add di, 2

		jmp .MESSAGE_LOOP
	
	.MESSAGE_END:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es

	pop bp
	ret 



MSG_FIRST: db 'Hello, World!', 0
MSG_DISK_ERROR: db '...Error', 0
MSG_IMAGE_LOADING: db 'Loading first kernel', 0
MSG_IMAGE_LOADED: db '...Complete', 0
	

SECTOR_NUMBER: db 0x02
HEAD_NUMBER: db 0x00
TRACK_NUMBER: db 0x00


times 510 - ( $ - $$ ) db 0x00

db 0x55
db 0xaa
