#include "pit.h"
#include "helper_asm.h"

void initialize_pit(WORD count, BOOL periodic) {
    out1(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

    if (periodic == TRUE) {
        out1(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
    }

    out1(PIT_PORT_COUNTER0, count & 0xFF);
    out1(PIT_PORT_COUNTER0, count >> 8);
}

WORD read_counter0(void) {
    BYTE high, low;

    out1(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);

    low = in1(PIT_PORT_COUNTER0);
    high = in1(PIT_PORT_COUNTER0);
    
    return (high << 8) | low;
}

void wait_using_direct_pit(WORD count) {
    WORD last_counter0;
    WORD cur_counter0;

    initialize_pit(0, TRUE);

    last_counter0 = read_counter0();
    while (1) {
        cur_counter0 = read_counter0();
        if ((last_counter0 - cur_counter0) >= count) {
            break;
        }
    }
}