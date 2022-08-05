#ifndef __keyboard_h__
#define __keyboard_h__

#include "types.h"

#define KEY_FLAGS_UP        0x00
#define KEY_FLAGS_DOWN      0x01
#define KEY_FLAGS_EXTENDED  0x02

#define KEY_TABLE_MAX       89

#define KEY_NONE        0x00
#define KEY_ENTER       '\n'
#define KEY_TAB         '\t'
#define KEY_ESC         0x1B
#define KEY_BACKSPACE   0x08
#define KEY_CTRL        0x81
#define KEY_LSHIFT      0x82
#define KEY_RSHIFT      0x83
#define KEY_PRINTSCREEN 0x84
#define KEY_LALT        0x85
#define KEY_CAPSLOCK    0x86
#define KEY_F1          0x87
#define KEY_F2          0x88
#define KEY_F3          0x89
#define KEY_F4          0x8a
#define KEY_F5          0x8b
#define KEY_F6          0x8c
#define KEY_F7          0x8d
#define KEY_F8          0x8e
#define KEY_F9          0x8f
#define KEY_F10         0x90
#define KEY_NUMLOCK     0x91
#define KEY_SCROLLLOCK  0x92
#define KEY_HOME        0x93
#define KEY_UP          0x94
#define KEY_PAGEUP      0x95
#define KEY_LEFT        0x96
#define KEY_CENTER      0x97
#define KEY_RIGHT       0x98
#define KEY_END         0x99
#define KEY_DOWN        0x9a
#define KEY_PAGEDOWN    0x9b
#define KEY_INS         0x9c
#define KEY_DEL         0x9d
#define KEY_F11         0x9e
#define KEY_F12         0x9f
#define KEY_PAUSE       0xa0

#pragma pack(push, 1)

struct keymap_entry {
    BYTE normal_code;
    BYTE combined_code;
};

#pragma pack(pop)

struct keyboard_manager {
    BOOL is_shift;
    BOOL is_capslock;
    BOOL is_numlock;
    BOOL is_scrolllock;

    BOOL is_extended;
    int skip_for_pause;
};

void wait_out(void);
void wait_in(void);
BOOL wait_ack(void);
BOOL is_output_buffer_ready(void);
BOOL is_input_buffer_ready(void);
BOOL activate_keyboard(void);
BYTE get_scan_code(void);
BOOL change_led(BOOL is_capslock, BOOL is_numlock, BOOL is_scrolllock);
void enable_a20(void);
void reboot(void);
BOOL is_alpha(BYTE scan_code);
BOOL is_num_or_special(BYTE scan_code);
BOOL is_numpad(BYTE scan_code);
BOOL is_combined(BYTE scan_code);
void update_keyboard_status(BYTE scan_code);
BYTE scancode_to_ascii(BYTE scan_code, BYTE *ascii, BOOL *flags);

static struct keymap_entry key_map_table[KEY_TABLE_MAX] = {
    /* 0x00 */  { KEY_NONE,         KEY_NONE },
    /* 0x01 */  { KEY_ESC,          KEY_ESC },
    /* 0x02 */  { '1',              '!' },
    /* 0x03 */  { '2',              '@' },
    /* 0x04 */  { '3',              '#' },
    /* 0x05 */  { '4',              '$' },
    /* 0x06 */  { '5',              '%' },
    /* 0x07 */  { '6',              '^' },
    /* 0x08 */  { '7',              '&' },
    /* 0x09 */  { '8',              '*' },
    /* 0x0a */  { '9',              '(' },
    /* 0x0b */  { '0',              ')' },
    /* 0x0c */  { '-',              '_' },
    /* 0x0d */  { '=',              '+' },
    /* 0x0e */  { KEY_BACKSPACE,    KEY_BACKSPACE },
    /* 0x0f */  { KEY_TAB,          KEY_TAB },
    /* 0x10 */  { 'q',              'Q' },
    /* 0x11 */  { 'w',              'W' },
    /* 0x12 */  { 'e',              'E' },
    /* 0x13 */  { 'r',              'R' },
    /* 0x14 */  { 't',              'T' },
    /* 0x15 */  { 'y',              'Y' },
    /* 0x16 */  { 'u',              'U' },
    /* 0x17 */  { 'i',              'I' },
    /* 0x18 */  { 'o',              'O' },
    /* 0x19 */  { 'p',              'P' },
    /* 0x1a */  { '[',              '{' },
    /* 0x1b */  { ']',              '}' },
    /* 0x1c */  { '\n',             '\n' },
    /* 0x1d */  { KEY_CTRL,         KEY_CTRL },
    /* 0x1e */  { 'a',              'A' },
    /* 0x1f */  { 's',              'S' },
    /* 0x20 */  { 'd',              'D' },
    /* 0x21 */  { 'f',              'F' },
    /* 0x22 */  { 'g',              'G' },
    /* 0x23 */  { 'h',              'H' },
    /* 0x24 */  { 'j',              'J' },
    /* 0x25 */  { 'k',              'K' },
    /* 0x26 */  { 'l',              'L' },
    /* 0x27 */  { ';',              ':' },
    /* 0x28 */  { '\'',             '\"' },
    /* 0x29 */  { '`',              '~' },
    /* 0x2a */  { KEY_LSHIFT,       KEY_LSHIFT },
    /* 0x2b */  { '\\',             '|' },
    /* 0x2c */  { 'z',              'Z' },
    /* 0x2d */  { 'x',              'X' },
    /* 0x2e */  { 'c',              'C' },
    /* 0x2f */  { 'v',              'V' },
    /* 0x30 */  { 'b',              'B' },
    /* 0x31 */  { 'n',              'N' },
    /* 0x32 */  { 'm',              'M' },
    /* 0x33 */  { ',',              '<' },
    /* 0x34 */  { '.',              '>' },
    /* 0x35 */  { '/',              '?' },
    /* 0x36 */  { KEY_RSHIFT,       KEY_RSHIFT },
    /* 0x37 */  { '*',              '*' },
    /* 0x38 */  { KEY_LALT,         KEY_LALT },
    /* 0x39 */  { ' ',              ' ' },
    /* 0x3a */  { KEY_CAPSLOCK,     KEY_CAPSLOCK },
    /* 0x3b */  { KEY_F1,           KEY_F1 },
    /* 0x3c */  { KEY_F2,           KEY_F2 },
    /* 0x3d */  { KEY_F3,           KEY_F3 },
    /* 0x3e */  { KEY_F4,           KEY_F4 },
    /* 0x3f */  { KEY_F5,           KEY_F5 },
    /* 0x40 */  { KEY_F6,           KEY_F6 },
    /* 0x41 */  { KEY_F7,           KEY_F7 },
    /* 0x42 */  { KEY_F8,           KEY_F8 },
    /* 0x43 */  { KEY_F9,           KEY_F9 },
    /* 0x44 */  { KEY_F10,          KEY_F10 },
    /* 0x45 */  { KEY_NUMLOCK,      KEY_NUMLOCK },
    /* 0x46 */  { KEY_SCROLLLOCK,   KEY_SCROLLLOCK },
    /* 0x47 */  { KEY_HOME,         '7' },
    /* 0x48 */  { KEY_UP,           '8' },
    /* 0x49 */  { KEY_PAGEUP,       '9' },
    /* 0x4a */  { '-',              '-' },
    /* 0x4b */  { KEY_LEFT,         '4' },
    /* 0x4c */  { KEY_CENTER,       '5' },
    /* 0x4d */  { KEY_RIGHT,        '6' },
    /* 0x4e */  { '+',              '+' },
    /* 0x4f */  { KEY_END,          '1' },
    /* 0x50 */  { KEY_DOWN,         '2' },
    /* 0x51 */  { KEY_PAGEDOWN,     '3' },
    /* 0x52 */  { KEY_INS,          '0' },
    /* 0x53 */  { KEY_DEL,          '.' },
    /* 0x54 */  { KEY_NONE,         KEY_NONE },
    /* 0x55 */  { KEY_NONE,         KEY_NONE },
    /* 0x56 */  { KEY_NONE,         KEY_NONE },
    /* 0x57 */  { KEY_F11,          KEY_F11 },    
    /* 0x58 */  { KEY_F12,          KEY_F12 },
};

#endif