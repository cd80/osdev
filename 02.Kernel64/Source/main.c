#include "types.h"
#include "keyboard.h"
#include "descriptor.h"
#include "pic.h"
#include "utility.h"
void progress(int x, int y, const char *string, BOOL success);
DWORD printat(int x, int y, const char *string);

void main(void) {
    progress(0, 10, "Switched to 64bit", TRUE);
    progress(0, 11, "Activate keyboard", activate_keyboard());
    progress(0, 12, "Initialize GDT & TSS for IA-32e", initialize_gdt_tss());
    progress(0, 13, "Initialize IDT", initialize_idt());
    progress(0, 14, "Initialize PIC & Interrupt", initialize_pic_and_interrupt());

    BYTE code;
    BYTE flags;
    int i = 0;
    BYTE ascii[2];
    ascii[1] = '\0';
    

    while(1) {
        wait_out();
        if(scancode_to_ascii(get_scan_code(), ascii, &flags)) {
            if(flags & KEY_FLAGS_DOWN) {
                if(ascii[0] == '0')  {
                    i = i / 0;
                }
                printat(i++, 16, (const char *)ascii);
            }
        }
    }
}

