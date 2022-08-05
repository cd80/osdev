#ifndef __descriptor_h__
#define __descriptor_h__

#include "types.h"

// GDT
#define GDT_TYPE_CODE           0x0A
#define GDT_TYPE_DATA           0x02
#define GDT_TYPE_TSS            0x09
#define GDT_FLAGS_LOWER_S       0x10
#define GDT_FLAGS_LOWER_DPL0    0x00
#define GDT_FLAGS_LOWER_DPL1    0x10
#define GDT_FLAGS_LOWER_DPL2    0x40
#define GDT_FLAGS_LOWER_DPL3    0x60
#define GDT_FLAGS_LOWER_P       0x80
#define GDT_FLAGS_UPPER_L       0x20
#define GDT_FLAGS_UPPER_DB      0x40
#define GDT_FLAGS_UPPER_G       0x80


#define GDT_FLAGS_LOWER_KERNELCODE  (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | \
                                    GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_KERNELDATA  (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | \
                                    GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_TSS         (GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERCODE    (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | \
                                    GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERDATA    (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | \
                                    GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)

#define GDT_FLAGS_UPPER_CODE        (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_DATA        (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_TSS         (GDT_FLAGS_UPPER_G)

#define GDT_KERNELCODESEGMENT   0x08
#define GDT_KERNELDATASEGMENT   0x10
#define GDT_TSSSEGMENT          0x18

#define GDTR_STARTADDRESS       0x142000
#define GDT_MAXENTRY8COUNT      3
#define GDT_MAXENTRY16COUNT     1
#define GDT_TABLESIZE ((sizeof(GDTENTRY8) * GDT_MAXENTRY8COUNT) + \
                        sizeof(GDTENTRY16) * GDT_MAXENTRY16COUNT)
#define TSS_SEGMENTSIZE (sizeof(TSSSEGMENT))


// IDT
#define IDT_TYPE_INTERRUPT  0x0E
#define IDT_TYPE_TRAP       0x0F
#define IDT_FLAGS_DPL0      0x00
#define IDT_FLAGS_DPL1      0x20
#define IDT_FLAGS_DPL2      0x40
#define IDT_FLAGS_DPL3      0x60
#define IDT_FLAGS_P         0x80
#define IDT_FLAGS_IST0      0
#define IDT_FLAGS_IST1      1

#define IDT_FLAGS_KERNEL    (IDT_FLAGS_DPL0 | IDT_FLAGS_P)
#define IDT_FLAGS_USER      (IDT_FLAGS_DPL3 | IDT_FLAGS_P)

#define IDT_MAXENTRYCOUNT   100
#define IDTR_STARTADDRESS   (GDTR_STARTADDRESS + sizeof(GDTR) + \
                            GDT_TABLESIZE + TSS_SEGMENTSIZE)

#define IDT_STARTADDRESS    (IDTR_STARTADDRESS + sizeof(IDTR))
#define IDT_TABLESIZE       (IDT_MAXENTRYCOUNT * sizeof(IDTENTRY))

#define IST_STARTADDRESS    0x700000
#define IST_SIZE            0x100000

#pragma pack(push, 1)
typedef struct _gdtr {
    WORD limit;
    QWORD base_address;
    WORD padding1;
    DWORD padding2;
} GDTR, IDTR;

typedef struct _gdtentry8 {
    WORD lower_limit;
    WORD lower_base;
    BYTE upper_base1;
    BYTE type_lower_flag;
    BYTE upper_limit_upper_flag;
    BYTE upper_base2;
} GDTENTRY8;

typedef struct _gdtentry16 {
    WORD lower_limit;
    WORD lower_base;
    BYTE middle_base1;
    BYTE type_lower_flag;
    BYTE upper_limit_upper_flag;
    BYTE middle_base2;
    DWORD upper_base;
    DWORD reserved;
} GDTENTRY16;

typedef struct _tss {
    DWORD reserved1;
    QWORD rsp[3];
    QWORD reserved2;
    QWORD ist[7];
    QWORD reserved3;
    WORD reserved4;
    WORD iomap_base;
} TSSSEGMENT;

typedef struct _idtentry {
    WORD lower_base;
    WORD segment_selector;
    BYTE ist;
    BYTE type_flags;
    WORD middle_base;
    DWORD upper_base;
    DWORD reserved;
} IDTENTRY;

#pragma pack(pop)

BOOL initialize_gdt_tss(void);
void set_gdt_entry8(GDTENTRY8 *entry, DWORD base_address, DWORD limit,
                    BYTE upper_flags, BYTE lower_flags, BYTE type);
void set_gdt_entry16(GDTENTRY16 *entry, QWORD base_address, DWORD limit,
                    BYTE upper_flags, BYTE lower_flags, BYTE type);
void initialize_tss(TSSSEGMENT *tss);
BOOL initialize_idt(void);
void set_idt_entry(IDTENTRY *entry, void *handler, WORD selector,
                    BYTE ist, BYTE flags, BYTE type);
void dummy_handler(void);

#endif