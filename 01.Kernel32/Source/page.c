#include "page.h"

BOOL init_page_tables(void) {
    QWORD mapping_addr;
    PDENTRY *pd_entry;
    PDPTENTRY *pdpt_entry;
    PML4TENTRY *pml4t_entry;
    
    pml4t_entry = (PML4TENTRY *)0x100000;
    set_page_entry(&pml4t_entry[0], 0x101000, PAGE_FLAGS_DEFAULT, 0);

    for(int i = 1; i < PAGE_MAXENTRYCOUNT; ++i) {
        set_page_entry(&(pml4t_entry[i]), 0, 0, 0);
    }

    pdpt_entry = (PDPTENTRY *)0x101000;
    for(int i = 0; i < 64; ++i) {
        set_page_entry(&pdpt_entry[i], 0x102000 + (i * PAGE_TABLESIZE),
                        PAGE_FLAGS_DEFAULT, 0);
    }

    for(int i = 64; i < PAGE_MAXENTRYCOUNT; ++i) {
        set_page_entry(&pdpt_entry[i], 0, 0, 0);
    }

    pd_entry = (PDENTRY *)0x102000;
    mapping_addr = 0;
    for(int i = 0; i < PAGE_MAXENTRYCOUNT * 64; ++i) {
        set_page_entry(&pd_entry[i], mapping_addr, 
                        PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
        mapping_addr += PAGE_DEFAULTSIZE;
    }

    return TRUE;
}

void set_page_entry(PTENTRY *entry, QWORD base_address, 
                    DWORD lower_flags, DWORD upper_flags) {
    entry->attrib_lowerbase = (base_address & 0xFFFFFFFF) | lower_flags;
    entry->upperbase_exb = ((base_address >> 32) & 0xFF) | upper_flags;
}