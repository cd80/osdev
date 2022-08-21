#ifndef __interrupt_handler_h__
#define __interrupt_handler_h__

#include "types.h"

void common_exception_handler(int vector, QWORD error_code);
void common_interrupt_handler(int vector);
void keyboard_handler(int vector);
void timer_handler(int vector);
void device_not_available_handler(int vector);

#endif