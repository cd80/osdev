#ifndef __utility_h__
#define __utility_h__

#include <stdarg.h>
#include "types.h"

void progress(const char *string, BOOL success);
DWORD printat(int x, int y, const char *string);
void memset(void *dest, BYTE data, unsigned int size);
unsigned int memcpy(void *dest, void *src, unsigned int size);
int memcmp(void *dest, void *src, unsigned int size);
BOOL set_interrupt_flag(BOOL enable_interrupt);
int strlen(const char *buf);
BOOL check_total_ram_size(void);
QWORD get_total_ram_size(void);
void reverse_string(char *buf);
long atoi(const char *buf, int radix);
QWORD hex_string_to_qword(const char *buf);
long dec_string_to_long(const char *buf);
int itoa(long value, char *buf, int radix);
int hex_to_string(QWORD value, char *buf);
int dec_to_string(long value, char *buf);
int sprintf(char *buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list ap);
QWORD get_tick_count(void);
void sleep(QWORD millisecond);

extern volatile QWORD g_tick_count;

#endif