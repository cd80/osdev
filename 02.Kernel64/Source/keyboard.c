#include "types.h"
#include "helper_asm.h"
#include "keyboard.h"
#include "queue.h"
#include "utility.h"
#include "sync.h"

struct keyboard_manager keyboard = {0, };
static struct generic_queue key_queue;
static struct keydata key_queue_buf[KEY_MAXQUEUECOUNT];

inline BOOL is_output_buffer_ready(void) {
    if (in1(0x64) & 0x01) {
        return TRUE;
    }
    return FALSE;
}

inline BOOL is_input_buffer_ready(void) {
    if(in1(0x64) & 0x02) {
        return FALSE;
    }
    return TRUE;
}

BOOL activate_keyboard(void) {
    // control register: 0x64, keyboard acitvate cmd: 0xae
    // activate keyboard in controller
    BOOL prev_interrupt;
    BOOL ack_result;
    prev_interrupt = lock_system();
    out1(0x64, 0xae);

    wait_in();

    // input buffer: 0x60, keyboard activate cmd: 0xf4
    // activate keyboard in keyboard
    out1(0x60, 0xf4);
    ack_result = wait_ack();

    unlock_system(prev_interrupt);
    return ack_result;
}

BYTE get_scan_code(void) {
    wait_out();

    return in1(0x60);
}

BOOL change_led(BOOL is_capslock, BOOL is_numlock, BOOL is_scrolllock) {
    BOOL prev_interrupt;
    BOOL ack_result;

    prev_interrupt = lock_system();

    wait_in();
    out1(0x60, 0xED);

    wait_in();
    if(!wait_ack()) {
        unlock_system(prev_interrupt);
        return FALSE;
    }

    out1(0x60, (is_capslock << 2) |
               (is_numlock << 1) |
               (is_scrolllock));
    wait_in();

    ack_result = wait_ack();
    
    unlock_system(prev_interrupt);
    return ack_result;
}

void enable_a20(void) {
    BYTE keyboard_status;
    out1(0x64, 0xD0);

    wait_out();
    keyboard_status = in1(0x60);
    keyboard_status |= 0x01;

    wait_in();
    out1(0x64, 0xD1);
    out1(0x60, keyboard_status);
}

void reboot(void) {
    wait_in();
    out1(0x64, 0xd1);
    out1(0x60, 0x00);

    while(TRUE) { }
}

BOOL is_alpha(BYTE scan_code) {
    if(('a' <= key_map_table[scan_code].normal_code) &&
        'z' >= key_map_table[scan_code].normal_code) {
        return TRUE;
    }
    return FALSE;
}

BOOL is_num_or_special(BYTE scan_code) {
    if((2 <= scan_code) && 
       (53 >= scan_code) &&
        is_alpha(scan_code) == FALSE) {
        return TRUE;
    }
    return FALSE;
}

BOOL is_numpad(BYTE scan_code) {
    if((71 <= scan_code) &&
       (83 >= scan_code)) {
        return TRUE;
    }
    return FALSE;
}

BOOL is_combined(BYTE scan_code) {
    BYTE down_code = scan_code & 0x7F;
    BOOL use_combined = FALSE;

    if(is_alpha(down_code) == TRUE){
        use_combined = keyboard.is_shift ^ keyboard.is_capslock;
    }
    else if(is_num_or_special(down_code)) {
        use_combined = keyboard.is_shift;
    }
    else if(is_numpad(down_code) && !keyboard.is_extended) {
        use_combined = keyboard.is_numlock;
    }

    return use_combined;
}

void update_keyboard_status(BYTE scan_code) {
    BYTE down_code;
    BOOL is_down;
    BOOL led_changed = TRUE;
    if(scan_code & 0x80) {
        is_down = FALSE;
        down_code = scan_code & 0x7F;
    }
    else {
        is_down = TRUE;
        down_code = scan_code;
    }

    if(down_code == 42 || down_code == 54) {
        keyboard.is_shift = is_down;
        led_changed = FALSE;
    }
    else if(down_code == 58 && is_down) {
        keyboard.is_capslock ^= TRUE;
    }
    else if(down_code == 69 && is_down) {
        keyboard.is_numlock ^= TRUE;
    }
    else if(down_code == 70 && is_down) {
        keyboard.is_scrolllock ^= TRUE;
    }
    else {
        led_changed = FALSE;
    }

    if(led_changed) {
        change_led(keyboard.is_capslock, keyboard.is_numlock, keyboard.is_scrolllock);
    }
}

BOOL scancode_to_ascii(BYTE scan_code, BYTE *ascii, BOOL *flags) {
    BOOL use_combined;

    if(keyboard.skip_for_pause > 0) {
        keyboard.skip_for_pause--;
        return FALSE;
    }

    if (scan_code == 0xE1) {
        *ascii = KEY_PAUSE;
        *flags = KEY_FLAGS_DOWN;
        keyboard.skip_for_pause = 2;
        return TRUE;
    }
    else if(scan_code == 0xE0) {
        keyboard.is_extended = TRUE;
        return FALSE;
    }

    use_combined = is_combined(scan_code);

    if(use_combined) {
        *ascii = key_map_table[scan_code & 0x7F].combined_code;
    }
    else {
        *ascii = key_map_table[scan_code & 0x7F].normal_code;
    }

    if(keyboard.is_extended) {
        *flags = KEY_FLAGS_EXTENDED;
        keyboard.is_extended = FALSE;
    }
    else {
        *flags = 0;
    }

    if((scan_code & 0x80) == 0) {
        *flags |= KEY_FLAGS_DOWN;
    }

    update_keyboard_status(scan_code);
    return TRUE;
}


void wait_out(void) {
    for(int i = 0; i < 0xFFFF; ++i){
        if(is_output_buffer_ready()) {
            break;
        }
    }
}
void wait_in(void) {
    for(int i = 0; i < 0xFFFF; ++i) {
        if(is_input_buffer_ready()) {
            break;
        }
    }
}

BOOL wait_ack(void) {
    BYTE data = 0;
    for(int i = 0; i < 0x100; ++i) {
        wait_out();
        data = in1(0x60);
        if(data == 0xfa) {
            return TRUE;
        }
        else {
            convert_scancode_and_put_queue(data);
        }
    }
    return FALSE;
}

BOOL initialize_keyboard(void) {
    BOOL activate_result = FALSE;
    initialize_queue(&key_queue, key_queue_buf, KEY_MAXQUEUECOUNT, 
                    sizeof(struct keydata));
    activate_result = activate_keyboard();
    if (activate_result) {
        change_led(FALSE, FALSE, FALSE);
    }

    return activate_result;
}

BOOL convert_scancode_and_put_queue(BYTE scan_code)  {
    BOOL result = FALSE;
    BOOL prev_interrupt;
    struct keydata data;

    data.scan_code = scan_code;

    if(scancode_to_ascii(scan_code, &(data.ascii_code), &data.flags)) {
        prev_interrupt = lock_system();
        result = put_queue(&key_queue, &data);
        unlock_system(prev_interrupt);
    }

    return result;
}
BOOL get_key_from_key_queue(struct keydata *data) {
    BOOL result;
    BOOL prev_interrupt;

    if(is_queue_empty(&key_queue) == TRUE) {
        return FALSE;
    }

    prev_interrupt = lock_system();
    result = get_queue(&key_queue, data);
    unlock_system(prev_interrupt);

    return result;
}