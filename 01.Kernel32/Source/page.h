#ifndef __page_h__
#define __page_h__

#include "types.h"

#define PAGE_FLAGS_P        0x00000001
#define PAGE_FLAGS_RW       0x00000002
#define PAGE_FLAGS_US       0x00000004
#define PAGE_FLAGS_PWT      0x00000008
#define PAGE_FLAGS_PCD      0x00000010
#define PAGE_FLAGS_A        0x00000020
#define PAGE_FLAGS_D        0x00000040
#define PAGE_FLAGS_PS       0x00000080
#define PAGE_FLAGS_G        0x00000100
#define PAGE_FLAGS_PAT      0x00001000
#define PAGE_FLAGS_EXP      0x80000000
#define PAGE_FLAGS_DEFAULT  ( PAGE_FLAGS_P | PAGE_FLAGS_RW )
#define PAGE_TABLESIZE      0x1000
#define PAGE_MAXENTRYCOUNT  512
#define PAGE_DEFAULTSIZE    0x200000 // 2MB

#pragma pack(push, 1)
typedef struct _PTEStruct {
    DWORD attrib_lowerbase;
    DWORD upperbase_exb;
} PML4TENTRY, PDPTENTRY, PDENTRY, PTENTRY;
#pragma pack(pop)

BOOL init_page_tables(void);
void set_page_entry(PTENTRY *entry, QWORD base_address, 
                    DWORD lower_flags, DWORD upper_flags);

#endif