#include "interrupt_handler.h"
#include "pic.h"
#include "keyboard.h"
#include "utility.h"
#include "console.h"
#include "task.h"
#include "descriptor.h"
#include "helper_asm.h"

void common_exception_handler(int vector, QWORD error_code) {
    char buf[3] = {0, };
    buf[0] = '0' + (vector / 10);
    buf[1] = '0' + (vector % 10);

    printat(0, 0, "===========================");
    printat(0, 1, "||      Exception!!!     ||");
    printat(0, 2, "||      Vector:          ||");
    printat(0, 3, "===========================");
    printat(16, 2, buf);

    while(1) { }
}

void common_interrupt_handler(int vector) {
    char buf[] = "[INT:  , ]";
    static int g_common_interrupt_count = 0;

    buf[5] = '0' + (vector / 10);
    buf[6] = '0' + (vector % 10);
    
    buf[8] = '0' + g_common_interrupt_count;
    g_common_interrupt_count = (g_common_interrupt_count + 1) % 10;
    printat(70, 0, buf);

    send_eoi_to_pic(vector - PIC_IRQSTARTVECTOR);
}

void keyboard_handler(int vector) {
    char buf[] = "[INT:  , ]";
    static int g_keyboard_interrupt_count = 0;
    BYTE temp;

    buf[5] = '0' + (vector / 10);
    buf[6] = '0' + (vector % 10);

    buf[8] = '0' + g_keyboard_interrupt_count;
    g_keyboard_interrupt_count = (g_keyboard_interrupt_count + 1) % 10;
    printat(0, 0, buf);

    if(is_output_buffer_ready()) {
        temp = get_scan_code();
        convert_scancode_and_put_queue(temp);
    }

    send_eoi_to_pic(vector - PIC_IRQSTARTVECTOR);
}

void timer_handler(int vector) {
    char buf[] = "[INT:  , ]";
    static int g_timer_interrupt_count = 0;

    buf[5] = '0' + (vector / 10);
    buf[6] = '0' + (vector % 10);
    
    buf[8] = g_timer_interrupt_count;
    g_timer_interrupt_count = '0' + (g_timer_interrupt_count + 1) % 10;
    printat(70, 0, buf);

    send_eoi_to_pic(vector - PIC_IRQSTARTVECTOR);

    g_tick_count++;

    decrease_processor_time();
    if (is_processor_time_expired() == TRUE) {
        schedule_in_interrupt();
    }
}

void device_not_available_handler(int vector) {
    TCB *fpu_task, *cur_task;
    QWORD last_fpu_task_id;

    char buf[] = "[EXC:  , ]";
    static int g_fpu_interrupt_count = 0;

    buf[5] = '0' + (vector / 10);
    buf[6] = '0' + (vector % 10);

    buf[8] = '0' + g_fpu_interrupt_count;
    g_fpu_interrupt_count = (g_fpu_interrupt_count + 1) % 10;
    printat(0, 0, buf);

    clear_ts();

    last_fpu_task_id = get_last_fpu_task_id();
    cur_task = get_running_task();

    if (last_fpu_task_id == cur_task->link.id) {
        return;
    }
    else if (last_fpu_task_id != TASK_INVALIDID) {
        fpu_task = get_tcb_in_tcb_pool(GETTCBOFFSET(last_fpu_task_id));
        if ((fpu_task != NULL) && (fpu_task->link.id == last_fpu_task_id)) {
            save_fpu_ctx(fpu_task->fpu_ctx);
        }
    }

    if (cur_task->fpu_used == FALSE) {
        init_fpu();
    }
    else {
        load_fpu_ctx(cur_task->fpu_ctx);
    }

    set_last_fpu_task_id(cur_task->link.id);
}