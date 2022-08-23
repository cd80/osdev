#include "types.h"
#include "keyboard.h"
#include "descriptor.h"
#include "pic.h"
#include "utility.h"
#include "console.h"
#include "console_shell.h"
#include "task.h"
#include "pit.h"
#include "dynamic_memory.h"
#include "harddisk.h"

void main(void) {
    progress(0, 10, "Switched to 64bit", TRUE);
    progress(0, 11, "Initialize GDT & TSS for IA-32e", initialize_gdt_tss());
    progress(0, 12, "Initialize IDT", initialize_idt());
    progress(0, 13, "Initialize PIC & Interrupt", initialize_pic_and_interrupt());
    progress(0, 14, "Initialize Keyboard", initialize_keyboard());
    progress(0, 15, "Initialize TCB Pool & Scheduler", initialize_scheduler());
    progress(0, 16, "Initialize Dynamic Memory", initialize_dynamic_memory());
    progress(0, 17, "Initialize HDD", initialize_hdd());
    initialize_pit(MSTOCOUNT(1), 1);


    initialize_console(0, 18);
    create_task(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)idle_task);
    start_console_shell();
}

