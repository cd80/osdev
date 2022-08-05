#include "pic.h"
#include "helper_asm.h"

void initialize_pic(void) {
    // ICW1(port 0x20), IC4(bit0) = 1
    out1(PIC_MASTER_PORT1, 0x11);

    // ICW2(port 0x21), Interrupt Vector 0x20
    out1(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

    // ICW3(port 0x21), slave PIC connected to pin #2 of master PIC, bit2 = 1
    out1(PIC_MASTER_PORT2, 0x02);

    // ICW4(port 0x21), uPM(bit0) = 1
    out1(PIC_MASTER_PORT2, 0x01);

    // Init slave PIC
    // ICW1(port 0xa0), IC4(bit0) = 1
    out1(PIC_SLAVE_PORT1, 0x11);

    // ICW2(port 0xa1), Interrupt Vector 0x20+8
    out1(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR + 8);

    // ICW3(port 0xa1), master #2 <-> slave
    out1(PIC_SLAVE_PORT2, 0x02);

    // ICW4(port 0xa1), uPM(bit0) = 1
    out1(PIC_SLAVE_PORT2, 0x01);
}

void mask_pic_interrupt(WORD irq_bit_mask) {
    // set master pic's IMR
    // OCW1(port 0x21), IRQ 0~7
    out1(PIC_MASTER_PORT2, (BYTE)irq_bit_mask);

    // set slave pic's IMR
    // OCW1(port 0xa1), IRQ 8~15
    out1(PIC_SLAVE_PORT2, (BYTE)(irq_bit_mask >> 8));
}

void send_eoi_to_pic(int irq_number) {
    // send EOI to master PIC
    // OCW2(port 0x20), EOI (bit5) = 1
    out1(PIC_MASTER_PORT1, 1 << 5);

    // send eoi to slave too if IRQ >= 8
    if (irq_number >= 8) {
        // OCW2(port 0xa0), EOI (bit5) = 1
        out1(PIC_SLAVE_PORT1, 1 << 5);
    }
}

BOOL initialize_pic_and_interrupt(void) {
    initialize_pic();
    mask_pic_interrupt(0);
    enable_interrupt();
    return TRUE;
}