#include "types.h"
#include "page.h"
#include "mode_switch.h"

void progress(int x, int y, const char *string, BOOL success);
DWORD printat(int x, int y, const char *string);
BOOL check_enough_mem(void);
BOOL init_kernel64_mem(void);
void get_vendor_string(char *);
BOOL check_64bit_support(void);
BOOL load_kernel64(void);

void main(void) {
    printat(0, 3, "Hello, C Kernel!");
    
    progress(0, 4, "Checking if there is enough memory", check_enough_mem());
    progress(0, 5, "Initializing IA-32e kernel memory", init_kernel64_mem());
    progress(0, 6, "Initializing IA-32e page tables", init_page_tables());

    char vendor_string[13];
    get_vendor_string(vendor_string);
    printat(0, 7, "Processor : ");
    printat(12, 7, vendor_string);

    progress(0, 8, "Checking if the processor supports 64bit mode", check_64bit_support());
    progress(0, 9, "Loading 64bit Kernel", load_kernel64());
    switch_to_64bit();
}

void progress(int x, int y, const char *string, BOOL success) {
    char *msg_success = success ? "PASS" : "FAIL";
    char attr = success ? 0x07 : 0x0c;
    DWORD str_len = 0;
    CHARACTER *screen = (CHARACTER *)0xB8000;
    screen += x + y * 80;
    str_len = printat(x, y, string);
    for(; str_len < 75; ++str_len) {
        screen[str_len].character = '.';
    }
    screen[74].character ='[';

    for(int i=0; msg_success[i] != 0; ++i) {
        screen[i+75].character = msg_success[i];
        screen[i+75].attr = attr;
    }

    screen[79].character = ']';
    
    if(msg_success[0] == 'F') {
        while (1) {}
    }
}

DWORD printat(int x, int y, const char *string) {
    DWORD str_len = 0;
    CHARACTER *screen = (CHARACTER *)0xB8000;
    screen += x + y * 80;
    for(int i=0; string[i] != 0; ++i) {
        str_len++;
        screen[i].character = string[i];
    }

    return str_len;
}

BOOL check_enough_mem() {
    DWORD *cur_addr = (DWORD*) 0x100000;
    while((DWORD)cur_addr < 64*1024*1024) {
        *cur_addr = 0xcd80cd80;

        if (*cur_addr != 0xcd80cd80) {
            return FALSE;
        }
        *cur_addr = 0x00000000;

        cur_addr += 1024*1024/sizeof(DWORD);
    }
    return TRUE;
}

BOOL init_kernel64_mem(void) {
    DWORD *cur_addr = (DWORD *)0x100000;

    while((DWORD)cur_addr < 0x600000) {
        *cur_addr = 0x00;

        if (*cur_addr != 0x00) {
            return FALSE;
        }
        cur_addr++;
    }

    return TRUE;
}

void get_vendor_string(char *vendor_string) {
    DWORD eax, ebx, ecx, edx;
    read_cpuid(0x00, &eax, &ebx, &ecx, &edx);
    *(DWORD *)vendor_string = ebx;
    *((DWORD *)vendor_string + 1) = edx;
    *((DWORD *)vendor_string + 2) = ecx;
}

BOOL check_64bit_support(void) {
    DWORD eax, ebx, ecx, edx;
    read_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    return !!(edx & (1 << 29));
}

BOOL load_kernel64(void) {
    WORD kernel32_sector_count, total_sector_count;
    DWORD *src, *dest;

    total_sector_count = *((WORD *)0x7c05);
    kernel32_sector_count = *((WORD *)0x7c07);

    src = (DWORD *)(0x10000 + (kernel32_sector_count * 512));
    dest = (DWORD *)0x200000;

    for(int i = 0; i < 512 * (total_sector_count - kernel32_sector_count) / 4; ++i) {
        *dest = *src;
        ++dest;
        ++src;
    }
    return TRUE;
}