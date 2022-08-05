#include "types.h"
#include "keyboard.h"
#include "descriptor.h"
#include "pic.h"
#include "utility.h"
void progress(int x, int y, const char *string, BOOL success);
DWORD printat(int x, int y, const char *string);

void main(void) {
    progress(0, 10, "Switched to 64bit", TRUE);
    progress(0, 11, "Initialize GDT & TSS for IA-32e", initialize_gdt_tss());
    progress(0, 12, "Initialize IDT", initialize_idt());
    progress(0, 13, "Initialize PIC & Interrupt", initialize_pic_and_interrupt());
    progress(0, 14, "Initialize Keyboard", initialize_keyboard());

    BYTE code;
    BYTE flags;
    int i = 0;
    BYTE ascii[2];
    ascii[1] = '\0';

    struct keydata key_data;
    

    while(1) {
        if(get_key_from_key_queue(&key_data)) {
            if(key_data.flags & KEY_FLAGS_DOWN) {
                ascii[0] = key_data.ascii_code;
                printat(i++, 16, (const char *)ascii);

                if(ascii[0] == '0') {
                    i = i / 0;
                }
            }
        }
    }
}

