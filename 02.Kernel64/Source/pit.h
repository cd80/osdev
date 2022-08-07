#ifndef __pit_h__
#define __pit_h__

#include "types.h"

#define PIT_FREQUENCY       1193182
#define MSTOCOUNT(x)        (PIT_FREQUENCY * (x) / 1000)
#define USTOCOUNT(x)        (PIT_FREQUENCY * (x) / 1000000)

#define PIT_PORT_CONTROL    0x43
#define PIT_PORT_COUNTER0   0x40
#define PIT_PORT_COUNTER1   0x41
#define PIT_PORT_COUNTER2   0x42

#define PIT_CONTROL_COUNTER0    0x00
#define PIT_CONTROL_COUNTER1    0x40
#define PIT_CONTROL_COUNTER2    0x80
#define PIT_CONTROL_LSBMSBRW    0x30
#define PIT_CONTROL_LATCH       0x00
#define PIT_CONTROL_MODE0       0x00
#define PIT_CONTROL_MODE2       0x04

#define PIT_CONTROL_BINARYCOUNTER   0x00
#define PIT_CONTROL_BCDCOUNTER      0x01

#define PIT_COUNTER0_ONCE   (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | \
                            PIT_CONTROL_MODE0 | PIT_CONTROL_BINARYCOUNTER)
#define PIT_COUNTER0_PERIODIC   (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | \
                                PIT_CONTROL_MODE2 | PIT_CONTROL_BINARYCOUNTER)
#define PIT_COUNTER0_LATCH  (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LATCH)

void initialize_pit(WORD count, BOOL periodic);
WORD read_counter0(void);
void wait_using_direct_pit(WORD count);

#endif