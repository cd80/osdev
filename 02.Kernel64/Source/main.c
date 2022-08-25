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
#include "filesystem.h"

void main(void) {
    BOOL gdt_tss_result = FALSE;
    BOOL idt_result = FALSE;
    gdt_tss_result = initialize_gdt_tss();
    idt_result = initialize_idt();
    
    initialize_console(0, 10);
    progress("Switched to 64bit", TRUE);
    progress("Initialize GDT & TSS", gdt_tss_result);
    progress("Initialize IDT", idt_result);
    progress("Initialize PIC & Interrupt", initialize_pic_and_interrupt());
    progress("Initialize Keyboard", initialize_keyboard());
    progress("Initialize TCB Pool & Scheduler", initialize_scheduler());
    create_task(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, (QWORD)idle_task);
    progress("Initialize Dynamic Memory", initialize_dynamic_memory());
    progress("Initialize HDD", initialize_hdd());
    progress("Initialize filesystem", initialize_file_system());
    
    initialize_pit(MSTOCOUNT(1), 1);

    start_console_shell();
}

