#ifndef __pic_h__
#define __pic_h__

#include "types.h"

#define PIC_MASTER_PORT1    0x20
#define PIC_MASTER_PORT2    0x21
#define PIC_SLAVE_PORT1     0xA0
#define PIC_SLAVE_PORT2     0xA1

#define PIC_IRQSTARTVECTOR  0x20

void initialize_pic(void);
void mask_pic_interrupt(WORD irq_bit_mask);
void send_eoi_to_pic(int irq_number);

BOOL initialize_pic_and_interrupt();
#endif