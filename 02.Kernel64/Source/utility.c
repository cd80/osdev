#include <stdarg.h>
#include "utility.h"
#include "helper_asm.h"
#include "console.h"

volatile QWORD g_tick_count = 0;

void progress(const char *string, BOOL success) {
    CHARACTER *screen = (CHARACTER *)0xB8000;
    char *msg_success = success ? "PASS" : "FAIL";
    char attr = success ? 0x07 : 0x0c;
    char buf[80] = {0, };
    DWORD str_len = 0;

    if (success != TRUE && success != FALSE) {
        // Wrong call to progress
        // ex) progress(0, 0, "haha", func); => progress(0, 0, "haha", func());
        for (int i = 0; i < 80; ++i) {
            screen[i].character = 0;
            screen[i].attr = 0x0c;
        }
        printat(0, 0, "wrong call to progress: ");
        printat(24, 0, string);
    }
    screen += x + y * 80;
    str_len = memcpy(buf, string, strlen(string));
    
    for(; str_len < 75; ++str_len) {
        buf[str_len].character = '.';
    }
    buf[74].character ='[';

    for(int i=0; msg_success[i] != 0; ++i) {
        buf[i+75].character = msg_success[i];
        buf[i+75].attr = attr;
    }

    buf[79].character = ']';

    printf("%s\n");
    
    if(msg_success[0] == 'F') {
        while (1) {}
    }
}


void memset(void *dest, BYTE data, unsigned int size) {
    for(int i = 0; i < size; ++i) {
        ((char *)dest)[i] = data;
    }
}

unsigned int memcpy(void *dest, void *src, unsigned int size) {
    for(int i = 0; i < size; ++i) {
        ((char *)dest)[i] = ((char *)src)[i];
    }

    return size;
}

int memcmp(void *dest, void *src, unsigned int size) {
    int diff = 0;
    for(int i = 0; i < size; ++i) {
        diff = ((char *)dest)[i] - ((char *)src)[i];
        if(diff) {
            return diff;
        }
    }
    return 0;
}

BOOL set_interrupt_flag(BOOL enable) {
    QWORD RFLAGS;

    RFLAGS = read_rflags();
    if (enable == TRUE) {
        enable_interrupt();
    }
    else {
        disable_interrupt();
    }

    return !!(RFLAGS & 0x0200);
}

int strlen(const char *buf) {
    int len;
    for (len = 0; buf[len] != '\0'; ++len) {}

    return len;
}

static int total_ram_MB = -1;
BOOL check_total_ram_size(void) {
    DWORD *cur_addr;
    DWORD prev_value;

    cur_addr = (DWORD *)0x4000000;

    while (1) {
        prev_value = *cur_addr;
        *cur_addr = 0x12345678;
        if (*cur_addr != 0x12345678) {
            break;
        }
        *cur_addr = prev_value;
        cur_addr += (0x400000 / sizeof(DWORD));
    }

    total_ram_MB = (QWORD)cur_addr / 0x100000;
    return TRUE;
}

QWORD get_total_ram_size(void) {
    if (total_ram_MB == -1) {
        check_total_ram_size();
    }
    return total_ram_MB;
}

long atoi(const char *buf, int radix) {
    long return_value;
    
    switch(radix) {
        case 16:
            return_value = hex_string_to_qword(buf);
            break;
        case 10:
        default:
            return_value = dec_string_to_long(buf);
            break;
    }
    return return_value;
}

QWORD hex_string_to_qword(const char *buf) {
    QWORD value = 0;
    for (int i = 0; buf[i] != '\0'; ++i) {
        value *= 16;
        if (('A' <= buf[i]) && (buf[i] <= 'F')) {
            value += (buf[i] - 'A') + 10;
        }
        else if (('a' <= buf[i]) && (buf[i] <= 'f')) {
            value += (buf[i] - 'a') + 10;
        }
        else if (('0' <= buf[i]) && (buf[i] <= '9')) {
            value += buf[i] - '0';
        }
        else {
            break;
        }
    }
    return value;
}

long dec_string_to_long(const char *buf) {
    long value = 0;
    int idx = (buf[0] == '-') ? 1 : 0;

    for (; buf[idx] != '\0'; ++idx) {
        value *= 10;
        if (('0' <= buf[idx]) && (buf[idx] <= '9')) {
            value += buf[idx] - '0';
        }
        else {
            break;
        }
    }

    if (buf[0] == '-') {
        value = -value;
    }

    return value;
}

int itoa(long value, char *buf, int radix) {
    int return_value;

    switch(radix) {
        case 16:
            return_value = hex_to_string(value, buf);
            break;
        case 10:
        default:
            return_value = dec_to_string(value, buf);
            break;
    }

    return return_value;
}

int hex_to_string(QWORD value, char *buf) {
    QWORD idx;
    QWORD cur_value;

    if (value == 0) {
        buf[0] ='0' + value;
        buf[1] = '\0';
        return 1;
    }

    for (idx = 0; value > 0; ++idx) {
        cur_value = value % 16;
        if (cur_value >= 10) {
            buf[idx] = 'A' + (cur_value - 10);
        }
        else {
            buf[idx] = '0' + cur_value;
        }

        value = value / 16;
    }

    buf[idx] = '\0';

    reverse_string(buf);
    return idx;
}

int dec_to_string(long value, char *buf) {
    long idx;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    if (value < 0 ) {
        idx = 1;
        buf[0] = '-';
        value = -value;
    }
    else {
        idx = 0;
    }

    for (; value > 0; ++idx) {
        buf[idx] = '0' + (value % 10);
        value = value / 10;
    }

    buf[idx] = '\0';
    
    if (buf[0] == '-') {
        reverse_string(&(buf[1]));
    }
    else {
        reverse_string(buf);
    }
    return idx;
}

void reverse_string(char *buf) {
    int length;
    char temp;

    length = strlen(buf);
    for (int i = 0; i < length / 2; ++i) {
        temp = buf[i];
        buf[i] = buf[length - 1 - i];
        buf[length - 1 - i] = temp;
    }
}

int sprintf(char *buf, const char *fmt, ...) {
    va_list ap;
    int return_value;

    va_start(ap, fmt);
    return_value = vsprintf(buf, fmt, ap);
    va_end(ap);

    return return_value;
}

int vsprintf(char *buf, const char *fmt, va_list ap) {
    char *copy_string;
    int copy_length;
    int buf_idx = 0;
    int fmt_length = 0;
    int double_length = 0;
    int long_value;
    QWORD qword_value;
    double double_value;

    fmt_length = strlen(fmt);
    for (int i = 0; i < fmt_length; ++i) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 's':
                    copy_string = (char *)(va_arg(ap, char *));
                    copy_length = strlen(copy_string);
                    memcpy(buf + buf_idx, copy_string, copy_length);
                    buf_idx += copy_length;
                    break;

                case 'c':
                    buf[buf_idx] = (char)(va_arg(ap, int));
                    buf_idx++;
                    break;
                
                case 'd':
                case 'i':
                    long_value = (int)(va_arg(ap, int));
                    buf_idx += itoa(long_value, buf + buf_idx, 10);
                    break;
                
                case 'x':
                case 'X':
                    qword_value = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
                    buf_idx += itoa(qword_value, buf + buf_idx, 16);
                    break;
                
                case 'q':
                case 'Q':
                case 'p':
                    qword_value = (QWORD)(va_arg(ap, QWORD));
                    buf_idx += itoa(qword_value, buf + buf_idx, 16);
                    break;

                case 'f':
                    double_value = (double)(va_arg(ap, double));
                    double_value += 0.005;
                    buf[buf_idx] = '0' + ((QWORD)(double_value * 100) % 10);
                    buf[buf_idx + 1] = '0' + ((QWORD)(double_value * 10) % 10);
                    buf[buf_idx + 2] = '.';

                    for (double_length = 0; ; ++double_length) {
                        if (((QWORD)double_value == 0) && (double_length != 0)) {
                            break;
                        }
                        buf[buf_idx + 3 + double_length] = '0' + ((QWORD)double_value % 10);
                        double_value = double_value / 10;
                    }
                    buf[buf_idx + 3 + double_length] = '\0';
                    reverse_string(buf + buf_idx);
                    buf_idx += 3 + double_length;
                    break;
                
                default:
                    buf[buf_idx] = fmt[i];
                    buf_idx++;
                    break;
            }
        }
        else {
            buf[buf_idx] = fmt[i];
            buf_idx++;
        }
    }

    buf[buf_idx] = '\0';
    return buf_idx;
}

QWORD get_tick_count(void) {
    return g_tick_count;
}

void sleep(QWORD millisecond) {
    QWORD last_tick_count;
    last_tick_count = g_tick_count;

    while ((g_tick_count - last_tick_count) <= millisecond) {
        schedule();
    }
}