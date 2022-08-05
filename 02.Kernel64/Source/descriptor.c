#include "descriptor.h"
#include "utility.h"
#include "helper_asm.h"
#include "isr.h"

BOOL initialize_gdt_tss(void) {
    GDTR *gdtr;
    GDTENTRY8 *entry;
    TSSSEGMENT *tss;

    gdtr = (GDTR *)GDTR_STARTADDRESS;
    entry = (GDTENTRY8 *)(GDTR_STARTADDRESS + sizeof(GDTR));
    gdtr->limit = GDT_TABLESIZE - 1;
    gdtr->base_address = (QWORD)entry;
    
    tss = (TSSSEGMENT *)((QWORD)entry + GDT_TABLESIZE);

    set_gdt_entry8(&(entry[0]), 0, 0, 0, 0, 0);
    set_gdt_entry8(&(entry[1]), 0, 0xFFFFF, GDT_FLAGS_UPPER_CODE,
                    GDT_FLAGS_LOWER_KERNELCODE, GDT_TYPE_CODE);
    set_gdt_entry8(&(entry[2]), 0, 0xFFFFF, GDT_FLAGS_UPPER_DATA,
                    GDT_FLAGS_LOWER_KERNELDATA, GDT_TYPE_DATA);
    set_gdt_entry16((GDTENTRY16 *)&(entry[3]), (QWORD)tss,
                sizeof(TSSSEGMENT)-1, GDT_FLAGS_UPPER_TSS, GDT_FLAGS_LOWER_TSS,
                GDT_TYPE_TSS);
    
    initialize_tss(tss);

    load_gdtr(GDTR_STARTADDRESS);
    load_tr(GDT_TSSSEGMENT);
    return TRUE;
}

void set_gdt_entry8(GDTENTRY8 *entry, DWORD base_address, DWORD limit,
                    BYTE upper_flags, BYTE lower_flags, BYTE type) {
    entry->lower_limit = limit & 0xFFFF;
    entry->lower_base = base_address & 0xFFFF;
    entry->upper_base1 = (base_address >> 16) & 0xFF;
    entry->type_lower_flag = lower_flags | type;
    entry->upper_limit_upper_flag = ((limit >> 16) & 0x0F) | upper_flags;
    entry->upper_base2 = (base_address >> 24) & 0xFF;
}

void set_gdt_entry16(GDTENTRY16 *entry, QWORD base_address, DWORD limit,
                    BYTE upper_flags, BYTE lower_flags, BYTE type) {
    entry->lower_limit = limit & 0xFFFF;
    entry->lower_base = base_address & 0xFFFF;
    entry->middle_base1 = (base_address >> 16) & 0xFF;
    entry->type_lower_flag = lower_flags | type;
    entry->upper_limit_upper_flag = ((limit >> 16) & 0x0F) | upper_flags;
    entry->middle_base2 = (base_address >> 24) & 0xFF;
    entry->upper_base = base_address >> 32;
    entry->reserved = 0;
}

void initialize_tss(TSSSEGMENT *tss) {
    memset(tss, 0, sizeof(TSSSEGMENT));
    tss->ist[0] = IST_STARTADDRESS + IST_SIZE;
    tss->iomap_base = 0xFFFF;
}

BOOL initialize_idt(void) {
    IDTR *idtr;
    IDTENTRY *entry;

    idtr = (IDTR *)IDTR_STARTADDRESS;
    entry = (IDTENTRY *)(IDTR_STARTADDRESS + sizeof(IDTR));
    idtr->base_address = (QWORD)entry;
    idtr->limit = IDT_TABLESIZE - 1;

    for(int i = 0; i < 21; ++i) {
        set_idt_entry(&(entry[i]), EXCEPTION_HANDLERS[i], 0x08, IDT_FLAGS_IST1, 
                    IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }
    for(int i = 21; i < 32; ++i) {
        set_idt_entry(&(entry[i]), isr_etc_exception, 0x08, IDT_FLAGS_IST1,
                    IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }
    for(int i = 32; i < 48; ++i) {
        set_idt_entry(&(entry[i]), INTERRUPT_HANDLERS[i-32], 0x08, IDT_FLAGS_IST1,
                    IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }
    for(int i = 48; i < IDT_MAXENTRYCOUNT; ++i) {
        set_idt_entry(&(entry[i]), isr_etc_interrupt, 0x08, IDT_FLAGS_IST1,
                    IDT_FLAGS_KERNEL, IDT_TYPE_INTERRUPT);
    }

    load_idtr(IDTR_STARTADDRESS);
    return TRUE;
}

void set_idt_entry(IDTENTRY *entry, void *handler, WORD segment_selector,
                    BYTE ist, BYTE flags, BYTE type) {
    entry->lower_base = (QWORD)handler & 0xFFFF;
    entry->segment_selector = segment_selector;
    entry->ist = ist & 0x03;
    entry->type_flags = type | flags;
    entry->middle_base = ((QWORD)handler >> 16) & 0xFFFF;
    entry->upper_base = (QWORD)handler >> 32;
    entry->reserved = 0;
 }

 void dummy_handler(void) {
    printat(0, 0, "============================");
    printat(0, 1, "||   IN DUMMY_HANDLER     ||");
    printat(0, 2, "============================");

    while(1) {}
 }