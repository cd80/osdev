#include <stdarg.h>
#include "console.h"
#include "keyboard.h"
#include "helper_asm.h"
#include "utility.h"

struct console_manager console_manager = {0, };

void initialize_console(int x, int y) {
    memset((void*)&console_manager, 0, sizeof(struct console_manager));
    set_cursor(x, y);
}

void set_cursor(int x, int y) {
    int linear_value = x + (y * CONSOLE_WIDTH);

    out1(VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR);
    out1(VGA_PORT_DATA, linear_value >> 8);

    out1(VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR);
    out1(VGA_PORT_DATA, linear_value & 0xFF);

    console_manager.cur_offset = linear_value;
}

void get_cursor(int *x, int *y) {
    *x = X(console_manager.cur_offset);
    *y = Y(console_manager.cur_offset);
}

void printf(const char *fmt, ...) {
    va_list ap;
    char buf[1024];
    int next_offset;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    next_offset = console_print(buf);

    set_cursor(X(next_offset), Y(next_offset));
}

int console_print(const char *buf) {
    CHARACTER *screen = (CHARACTER *)CONSOLE_VIDEOMEM;
    int buf_len = 0;
    int print_offset;
    print_offset = console_manager.cur_offset;

    buf_len = strlen(buf);
    for (int i = 0; i < buf_len; ++i) {
        if (buf[i] == '\n') {
            print_offset += (CONSOLE_WIDTH - X(print_offset));
        }
        else if (buf[i] == '\t') {
            print_offset += (4 - (print_offset % 4));
        }
        else {
            screen[print_offset].character = buf[i];
            screen[print_offset].attr = CONSOLE_DEFAULTTEXTCOLOR;
            print_offset++;
        }

        if (print_offset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH)) {
            memcpy(CONSOLE_VIDEOMEM,
                    CONSOLE_VIDEOMEM + CONSOLE_WIDTH * sizeof(CHARACTER),
                    (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

            for (int i = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
                i < (CONSOLE_HEIGHT * CONSOLE_WIDTH); ++i) {
                    screen[i].character = ' ';
                    screen[i].attr = CONSOLE_DEFAULTTEXTCOLOR;
            }
            print_offset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
        }
    }
    return print_offset;
}

void clear_screen(void) {
    CHARACTER *screen = (CHARACTER *)CONSOLE_VIDEOMEM;
    for (int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; ++i) {
        screen[i].character = ' ';
        screen[i].attr = CONSOLE_DEFAULTTEXTCOLOR;
    }
    set_cursor(0, 0);
}

BYTE getch(void) {
    struct keydata data;
    while(1) {
        while(get_key_from_key_queue(&data) == FALSE) { }

        if(data.flags & KEY_FLAGS_DOWN) {
            return data.ascii_code;
        }
    }
}

DWORD printat(int x, int y, const char *string) {
    DWORD str_len = 0;
    CHARACTER *screen = (CHARACTER *)0xB8000;
    screen += x + y * 80;
    for(int i=0; string[i] != 0; ++i) {
        str_len++;
        screen[i].character = string[i];
        screen[i].attr = CONSOLE_DEFAULTTEXTCOLOR;
    }

    return str_len;
}