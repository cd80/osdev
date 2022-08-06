#include "types.h"
#include "keyboard.h"
#include "descriptor.h"
#include "pic.h"
#include "utility.h"
#include "console.h"
#include "console_shell.h"

void main(void) {
    progress(0, 10, "Switched to 64bit", TRUE);
    progress(0, 11, "Initialize GDT & TSS for IA-32e", initialize_gdt_tss());
    progress(0, 12, "Initialize IDT", initialize_idt());
    progress(0, 13, "Initialize PIC & Interrupt", initialize_pic_and_interrupt());
    progress(0, 14, "Initialize Keyboard", initialize_keyboard());


    initialize_console(0, 16);
    start_console_shell();
    printf("TEST!!!!\n");
}

