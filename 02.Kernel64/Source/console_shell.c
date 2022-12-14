#include "console_shell.h"
#include "console.h"
#include "keyboard.h"
#include "utility.h"
#include "pit.h"
#include "rtc.h"
#include "helper_asm.h"
#include "task.h"
#include "sync.h"
#include "dynamic_memory.h"
#include "harddisk.h"
#include "filesystem.h"

struct shell_command_entry command_table[] = {
    { "help", "Show help", cmd_help },
    { "cls", "Clear screen", cmd_cls },
    { "totalram", "Show total RAM size", cmd_totalram },
    { "strtod", "Convert string to decimal/hex", cmd_strtod },
    { "reboot", "Reboot OS", cmd_reboot },
    { "settimer", "Set PIT controller counter0\n\t\t\t\tex) settimer 10(ms) 1(periodic)", cmd_settimer },
    { "wait", "Wait ms using PIT\n\t\t\t\tex)wait 100(ms)", cmd_wait },
    { "rdtsc", "Read TSC(Time Stamp Counter)", cmd_rdtsc },
    { "cpuspeed", "Measure processor speed", cmd_cpuspeed },
    { "date", "Show date and time", cmd_date },
    { "createtask", "Create Task, ex)createtask 1(type) 10(count)", cmd_createtask },
    { "changepriority", "Change task priority, ex) changepriority 1(ID) 2(Priority)", cmd_changepriority },
    { "tasklist", "Show task list", cmd_tasklist },
    { "killtask", "End Task, ex) killtask 1(ID) or 0xffffffff(All task)", cmd_killtask },
    { "cpuload", "Show processor load", cmd_cpuload },
    { "testmutex", "Test mutex function", cmd_testmutex },
    { "testthread", "Test thread and process", cmd_testthread },
    { "showmatrix", "Show matrix screen", cmd_showmatrix },
    { "testpi", "Test pi calculation", cmd_testpi },
    { "dynamicmeminfo", "Show dynamic memory information", cmd_dynamicmeminfo },
    { "testseqalloc", "Test sequential allocation & free", cmd_testseqalloc },
    { "testranalloc", "Test random allocation & free", cmd_testranalloc },
    { "hddinfo", "Show HDD information", cmd_hddinfo },
    { "readsector", "Read HDD sector, ex)readsector 0(LBA) 10(count)", cmd_readsector },
    { "writesector", "Write HDD sector, ex)writesector 0(LBA) 10(count)", cmd_writesector },
    { "testsleep", "Test sleep performance", cmd_testsleep },
    { "mounthdd", "Mount HDD", cmd_mounthdd },
    { "formathdd", "Format HDD", cmd_formathdd },
    { "filesysteminfo", "Show filesystem information", cmd_filesysteminfo },
    { "createfile", "Create file, ex)createfile a.txt", cmd_createfile },
    { "deletefile", "Delete file, ex)deletefile a.txt", cmd_deletefile },
    { "dir", "Show directory", cmd_dir },
};

void start_console_shell(void) {
    // Main loop of the shell
    char cmd_buf[CONSOLESHELL_MAXCMDBUFCOUNT] = {0, };
    int cmdbuf_idx = 0;
    BYTE key;
    int x, y;
    
    printf(CONSOLESHELL_PROMPT);
    while (1) {
        key = getch();

        switch (key) {
            case KEY_BACKSPACE:
                if (cmdbuf_idx > 0) {
                    get_cursor(&x, &y);
                    printat(x-1, y, " ");
                    set_cursor(x-1, y);
                    cmdbuf_idx--;
                }
                break;

            case KEY_ENTER:
                printf("\n");
                if (cmdbuf_idx > 0) {
                    cmd_buf[cmdbuf_idx] = '\0';
                    execute_command(cmd_buf);
                }

                printf(CONSOLESHELL_PROMPT);
                memset(cmd_buf, 0, CONSOLESHELL_MAXCMDBUFCOUNT);
                cmdbuf_idx = 0;
                break;

            case KEY_LSHIFT:
            case KEY_RSHIFT:
            case KEY_CAPSLOCK:
            case KEY_NUMLOCK:
            case KEY_SCROLLLOCK:
                break;
            
            default:
                if (key == KEY_TAB) {
                    key = ' ';
                }
                if (cmdbuf_idx < CONSOLESHELL_MAXCMDBUFCOUNT) {
                    cmd_buf[cmdbuf_idx++] = key;
                    printf("%c", key);
                }
                break;
        }
    }
}

void execute_command(const char *cmd_buf) {
    int space_idx;
    int cmdbuf_len, cmd_len;
    int count;

    cmdbuf_len = strlen(cmd_buf);
    for (space_idx = 0; space_idx < cmdbuf_len; ++space_idx) {
        if (cmd_buf[space_idx] == ' ') {
            break;
        }
    }

    count = sizeof(command_table) / sizeof(struct shell_command_entry);
    for (int i = 0; i < count; ++i){
        cmd_len = strlen(command_table[i].cmd);
        if ((cmd_len == space_idx) &&
            (memcmp(command_table[i].cmd, (void *)cmd_buf, space_idx) == 0)) {
            command_table[i].handler(cmd_buf + space_idx + 1);
            return;
        }
    }
    printf("Cannot execute: %s\n", cmd_buf);
}

void initialize_parameter(struct parameter_list *list, const char *param) {
    list->buf = param;
    list->length = strlen(param);
    list->cur_pos = 0;
}

int get_next_param(struct parameter_list *list, char *param) {
    int i;
    int length = 0;
    if (list->length <= list->cur_pos) {
        return 0;
    }

    for (i = list->cur_pos; i < list->length; ++i) {
        if (list->buf[i] == ' ') {
            break;
        }
    }

    memcpy(param, (void *)(list->buf + list->cur_pos), i);
    length = i - list->cur_pos;
    param[length] = '\0';

    list->cur_pos += length + 1;
    return length;
}

static void cmd_help(const char *param) {
    int count;
    int x, y;
    int length, max_cmd_length = 0;

    printf("========================================\n");
    printf("||               HELP                 ||\n");
    printf("========================================\n");

    count = sizeof(command_table) / sizeof(struct shell_command_entry);
    for (int i = 0; i < count; ++i) {
        length = strlen(command_table[i].cmd);
        if (length > max_cmd_length) {
            max_cmd_length = length;
        }
    }

    for (int i = 0; i < count; ++i) {
        printf("%s", command_table[i].cmd);
        get_cursor(&x, &y);
        set_cursor(max_cmd_length, y);
        printf(" - %s\n", command_table[i].help_msg);

        if ((i != 0) && ((i % 20) == 0)) {
            printf("Press any key to continue... ('q' is exit) : ");
            if (getch() == 'q') {
                printf("\n");
                break;
            }
            printf("\n");
        }
    }
}

static void cmd_cls(const char *param) {
    clear_screen();
    set_cursor(0, 1);
}

static void cmd_totalram(const char *param) {
    printf("Total RAM size = %d MB\n", get_total_ram_size());
}

static void cmd_strtod(const char *param) {
    char cur_param[100];
    int length;
    struct parameter_list param_list;
    int count = 0;
    long value;

    initialize_parameter(&param_list, param);

    while (1) {
        length = get_next_param(&param_list, cur_param);
        if (length == 0) {
            break;
        }

        printf("param %d = '%s', length = %d, ", count + 1, cur_param, length);

        if (memcmp((void *)param, "0x", 2) == 0) {
            value = atoi(cur_param + 2, 16);
            printf("HEX Value = %q\n", value);
        }
        else {
            value = atoi(cur_param, 10);
            printf("Decimal Value = %d\n", value);
        }

        count++;
    }
}

static void cmd_reboot(const char *param) {
    printf("[*] System reboot\n");
    printf("press any key to restart...");
    getch();
    reboot();
}

static void cmd_settimer(const char *param) {
    char cur_param[100];
    struct parameter_list param_list;
    long value;
    BOOL is_periodic;

    initialize_parameter(&param_list, param);

    if (get_next_param(&param_list, cur_param) == 0) {
        printf("ex) settimer 10(ms) 1(periodic)\n");
        return;
    }
    value = atoi(cur_param, 10);

    if (get_next_param(&param_list, cur_param) == 0) {
        printf("ex) settimer 10(ms) 1(periodic\n");
        return;
    }

    is_periodic = atoi(cur_param, 10);

    initialize_pit(MSTOCOUNT(value), is_periodic);
    printf("Time = %dms, Periodic = %d Change Complete\n", value, is_periodic);
}

static void cmd_wait(const char *param) {
    char cur_param[100];
    int length;
    struct parameter_list param_list;
    long millisecond;
    
    initialize_parameter(&param_list, param);

    if (get_next_param(&param_list, cur_param) == 0) {
        printf("ex) wait 100(ms)\n");
        return;
    }

    millisecond = atoi(cur_param, 10);
    printf("Starting %dms sleep\n", millisecond);

    disable_interrupt();
    for (int i = 0; i < millisecond / 30; ++i) {
        wait_using_direct_pit(MSTOCOUNT(30));
    }
    wait_using_direct_pit(MSTOCOUNT(millisecond % 30));
    enable_interrupt();

    printf("Finished %dms sleep\n", millisecond);

    initialize_pit(MSTOCOUNT(1), TRUE);
}

static void cmd_rdtsc(const char *param) {
    QWORD tsc;
    tsc = read_tsc();
    printf("Timestamp counter = %q\n", tsc);
}

static void cmd_cpuspeed(const char *param) {
    QWORD last_tsc, total_tsc = 0;

    printf("Start measuing for 10 seconds");
    
    disable_interrupt();
    for (int i = 0; i < 200; ++i) {
        last_tsc = read_tsc();
        wait_using_direct_pit(MSTOCOUNT(50));
        total_tsc += read_tsc() - last_tsc;

        if(i % 20 == 0) {
            printf(".");
        }
    }

    initialize_pit(MSTOCOUNT(1), TRUE);
    enable_interrupt();

    printf("\n\n");
    printf("CPU speed = %dMHz\n", total_tsc / 10 / 1000 / 1000);
}

static void cmd_date(const char *param) {
    BYTE second, minute, hour;
    BYTE day_of_week, day_of_month, month;
    WORD year;

    read_rtc_time(&hour, &minute, &second);
    read_rtc_date(&year, &month, &day_of_month, &day_of_week);

    printf("Date: %d/%d/%d %s, ", year, month, day_of_month,
                                convert_day_to_string(day_of_week));
    printf("Time: %d:%d:%d\n", hour, minute, second);
}

static void test_task1(void) {
    BYTE data;
    int i=0, x=0, y=0, margin;
    CHARACTER *screen = (CHARACTER *)CONSOLE_VIDEOMEM;
    TCB *running_task;

    running_task = get_running_task();
    margin = (running_task->link.id & 0xFFFFFFFF) % 10;

    for (int j = 0; j < 20000; ++j) {
        switch(i) {
            case 0:
                x++;
                if (x >= (CONSOLE_WIDTH - margin)) {
                    i = 1;
                }
                break;
            case 1:
                y++;
                if (y >= (CONSOLE_HEIGHT - margin)) {
                    i = 2;
                }
                break;
            case 2:
                x--;
                if (x < margin) {
                    i = 3;
                }
                break;
            case 3:
                y--;
                if (y < margin) {
                    i = 0;
                }
                break;
        }
        screen[x + y * CONSOLE_WIDTH].character = data;
        screen[x + y * CONSOLE_WIDTH].attr = data & 0x0F;
        data++;
    }

    exit_task();
}

static void test_task2(void) {
    int i = 0, offset;
    CHARACTER *screen = (CHARACTER *)CONSOLE_VIDEOMEM;
    TCB *running_task;
    char data[4] = {'-', '\\', '|', '/'};

    running_task = get_running_task();
    offset = (running_task->link.id & 0xFFFFFFFF) * 2;
    offset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (offset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while (1) {
        screen[offset].character = data[i % 4];
        screen[offset].attr = (offset % 15) + 1;
        i++;
    }
}

static void cmd_createtask(const char *param) {
    struct parameter_list param_list;
    char type[30];
    char count[30];
    int i;

    initialize_parameter(&param_list, param);
    get_next_param(&param_list, type);
    get_next_param(&param_list, count);

    switch(atoi(type, 10)) {
        case 1:
            for (i = 0; i < atoi(count, 10); ++i) {
                if (create_task(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)test_task1) == NULL) {
                    break;
                }
            }
            printf("Task1 %d Created\n", i);
            break;
        case 2:
        default:
            for (i = 0; i < atoi(count, 10); ++i) {
                if (create_task(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)test_task2) == NULL) {
                    break;
                }
            }
            printf("Task2 %d Created\n", i);
            break;
    }
}

static void cmd_changepriority(const char *param) {
    struct parameter_list param_list;
    char param_id[30];
    char param_priority[30];
    QWORD id;
    BYTE priority;

    initialize_parameter(&param_list, param);
    get_next_param(&param_list, param_id);
    get_next_param(&param_list, param_priority);

    if (memcmp(param_id, "0x", 2) == 0) {
        id = atoi(param_id + 2, 16);
    }
    else {
        id = atoi(param_id, 10);
    }

    priority = atoi(param_priority, 10);

    printf("Change task priority ID [0x%q] Priority[%d] ", id, priority);
    if (change_priority(id, priority) == TRUE) {
        printf("Success\n");
    }
    else {
        printf("Fail\n");
    }
}

static void cmd_tasklist(const char *param) {
    TCB *tcb;
    int count = 0;

    printf("============= Task Total Count [%d] =============\n", get_task_count());
    for (int i = 0; i < TASK_MAXCOUNT; ++i) {
        tcb = get_tcb_in_tcb_pool(i);
        if ((tcb->link.id >> 32) != 0) {
            if ((count != 0) && ((count % 10) == 0)) {
                printf("Press any key to continue... ('q' is exit) : ");
                if (getch() == 'q') {
                    printf("\n");
                    break;
                }
                printf("\n");
            }

            printf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n", 1 + count++,
                                            tcb->link.id, GETPRIORITY(tcb->flags),
                                            tcb->flags, get_list_count(&(tcb->child_thread_list)));
            printf("    Parent PID[0x%Q], MemoryAddress[0x%Q], Size[0x%Q]\n",
                        tcb->parent_pid, tcb->memory_address, tcb->memory_size);
        }
    }
}

static void cmd_killtask(const char *param) {
    struct parameter_list param_list;
    char param_id[30];
    QWORD id;
    TCB *tcb;

    initialize_parameter(&param_list, param);
    get_next_param(&param_list, param_id);
    
    if (memcmp(param_id, "0x", 2) == 0) {
        id = atoi(param_id + 2, 16);
    }
    else {
        id = atoi(param_id, 10);
    }

    if (id != 0xFFFFFFFF) {
        tcb = get_tcb_in_tcb_pool(GETTCBOFFSET(id));
        id = tcb->link.id;
        if (((id >> 32) != 0) && ((tcb->flags & TASK_FLAGS_SYSTEM) == 0x00)) {
            printf("Kill Task ID [0x%q] ", id);
            if (end_task(id) == TRUE) {
                printf("Success\n");
            }
            else {
                printf("Fail\n");
            }
        }
        else {
            printf("Task does not exist or task is system task\n");
        }
    }
    else {
        for (int i = 2; i < TASK_MAXCOUNT; ++i) {
            tcb = get_tcb_in_tcb_pool(i);
            id = tcb->link.id;
            if ((id >> 32) != 0 && ((tcb->flags & TASK_FLAGS_SYSTEM) == 0x00)) {
                printf("Kill Task ID [0x%q] ", id);
                if (end_task(id) == TRUE) {
                    printf("Success\n");
                }
                else {
                    printf("Fail\n");
                }
            }
        }
    }
}

static void cmd_cpuload(const char *param) {
    printf("Processor Load : %d%%\n", get_processor_load());
}

static MUTEX lock;
static volatile QWORD g_adder;

static void print_number_task(void) {
    QWORD tick_count;
    tick_count = get_tick_count();
    while ((get_tick_count() - tick_count) < 50) {
        schedule();
    }

    for (int i = 0; i < 5; ++i) {
        mutex_lock(&(lock));
        printf("Task ID [0x%Q] Value[%d]\n", get_running_task()->link.id, g_adder);

        g_adder += 1;
        mutex_unlock(&(lock));

        for (int j = 0; j < 300000; ++j) { }
    }

    tick_count = get_tick_count();
    while ((get_tick_count() - tick_count) < 1000) {
        schedule();
    }
    exit_task();
}

static void cmd_testmutex(const char *param) {
    int i;
    g_adder = 1;
    init_mutex(&lock);

    for (int i = 0; i < 3; ++i) {
        create_task(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)print_number_task);
    }

    printf("Wait until %d task end...\n", i);
    getch();
}

static void create_thread_task(void) {
    for (int i = 0; i < 3; ++i) {
        create_task(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD)test_task2);
    }

    while (1) {
        sleep(1);
    }
}

static void cmd_testthread(const char *param) {
    TCB *process;
    process = create_task(TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, (void *)0xEEEEEEEE, 0x1000, (QWORD)create_thread_task);

    if (process != NULL) {
        printf("Process [0x%Q] created\n", process->link.id);
    }
    else {
        printf("Create process fail\n");
    }
}

static volatile QWORD g_random_value = 0;

QWORD random(void) {
    g_random_value = (g_random_value * 412153 + 5571031) >> 16;
    return g_random_value;
}

static void drop_character_thread(void) {
    int x, y;
    char text[2] = {0, };

    x = random() % CONSOLE_WIDTH;

    while (1) {
        sleep(random() % 20);
        if ((random() % 20) < 15) {
            text[0] = ' ';
            for (int i = 0; i < CONSOLE_HEIGHT - 1; ++i) {
                printat(x, i, text);
                sleep(50);
            }
        }
        else {
            for (int i = 0 ; i < CONSOLE_HEIGHT - 1; ++i) {
                text[0] = i + random();
                printat(x, i, text);
                sleep(50);
            }
        }
    }
}

static void matrix_process(void) {
    int i;
    for (i = 0; i < 300; ++i) {
        if (create_task(TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0,
                        (QWORD)drop_character_thread) == NULL) {
            break;
        }
        sleep((random() % 5) + 5);
    }
    printf("  %d thread is created\n", i);
    getch();
}

static void cmd_showmatrix(const char *param) {
    TCB *process;
    process = create_task(TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, (void *)0xe00000, 0xe00000,
                            (QWORD)matrix_process);
    if (process != NULL) {
        printf("Matrix process [0x%Q] created successfully\n", process->link.id);

        while ((process->link.id >> 32) != 0) {
            sleep(100);
        }
    }
    else {
        printf("Failed to create matrix process\n");
    }
}

static void fpu_test_task(void) {
    double value1, value2;
    TCB *running_task;
    QWORD count = 0;
    QWORD random_value;
    int offset;
    char data[4] = {'-', '\\', '|', '/'};
    CHARACTER *screen = (CHARACTER *)CONSOLE_VIDEOMEM;

    running_task = get_running_task();

    offset = (running_task->link.id & 0xFFFFFFFF) * 2;
    offset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (offset % (CONSOLE_WIDTH * CONSOLE_HEIGHT)) + 2;

    while (1) {
        value1 = 1;
        value2 = 1;

        for (int i = 0; i < 10; ++i) {
            random_value = random();
            value1 *= (double)random_value;
            value2 *= (double)random_value;

            sleep(1);

            random_value = random();
            value1 /= (double)random_value;
            value2 /= (double)random_value;
        }

        if (value1 != value2) {
            printf("Value mismatch [%f] != [%f]\n", value1, value2);
            break;
        }
        count++;
        screen[offset].character = data[count%4];
        screen[offset].attr = (offset%15) + 1;
    }
}

static void cmd_testpi(const char *param) {
    double result;
    printf("PI calculation test\n");
    printf("Result: 355 / 113 = ");
    result = (double)355 / 113;
    printf("%d.%d%d\n", (QWORD) result, ((QWORD)(result * 10) % 10), ((QWORD)(result * 100) % 10));

    for (int i = 0; i < 100; ++i) {
        create_task(TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, (QWORD) fpu_test_task);
    }
}

static void cmd_dynamicmeminfo(const char *param) {
    QWORD start_address, total_size, meta_size, used_size;
    get_dynamic_memory_information(&start_address, &total_size,
                                    &meta_size, &used_size);
    printf("============ Dynamic Memory Information ============\n");
    printf("Start address : [0x%Q]\n", start_address);
    printf("Total size :    [0x%Q]bytes, [%d]MB\n", total_size, (total_size / 1024 / 1024));
    printf("Meta size:      [0x%Q]bytes, [%d]KB\n", meta_size, (meta_size / 1024));
    printf("Used size:      [0x%Q]bytes, [%d]KB\n", used_size, (used_size / 1024));
}

static void cmd_testseqalloc(const char *param) {
    DYNAMICMEMORYMANAGER *memory;
    QWORD *buf;

    printf("============ Dynamic Memory Test ============\n");
    memory = get_dynamic_memory_manager();

    for (long i = 0; i < memory->max_level_count; ++i) {
        printf("Block list [%d] test start\n", i);
        printf("Allocate and Compare: ");

        for (long j = 0; j < (memory->block_count_of_smallest_block >> i); ++j) {
            buf = allocate_memory(DYNAMICMEMORY_MIN_SIZE << i);
            if (buf == NULL) {
                printf("Allocation fail\n");
                return;
            }

            for (long k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; ++k) {
                buf[k] = k;
            }
            for (long k = 0; k < (DYNAMICMEMORY_MIN_SIZE << i) / 8; ++k) {
                if (buf[k] != k) {
                    printf("Compare fail\n");
                    return;
                }
            }
            printf(".");
        }

        printf("\nFree: ");
        for (long j = 0; j < (memory->block_count_of_smallest_block >> i); ++j) {
            if (free_memory((void *)(memory->start_address +
                                (DYNAMICMEMORY_MIN_SIZE << i) * j)) == FALSE) {
                printf("\nFree fail\n");
                return;
            }
            printf(".");
        }
        printf("\n");
    }
    printf("Test complete!!!\n");
}

static void random_alloc_task(void) {
    TCB *task;
    QWORD memory_size;
    char buf[200];
    BYTE *alloc_buf;
    int y;

    task = get_running_task();
    y = (task->link.id % 15) + 9;

    for (int i = 0; i < 10; ++i) {
        do {
            memory_size = ((random() % (32 * 1024)) + 1) * 1024;
            alloc_buf = allocate_memory(memory_size);
            
            if (alloc_buf == 0) {
                sleep(1);
            }
        } while (alloc_buf == 0);

        sprintf(buf, "|Address: [0x%Q] Size: [0x%Q] allocate success                    ",
                        alloc_buf, memory_size);
        printat(10, y, buf);
        sleep(200);

        sprintf(buf, "|Address: [0x%Q] Size: [0x%Q] write data...                       ",
                        alloc_buf, memory_size);
        printat(10, y, buf);

        for (int j = 0; j < memory_size / 2; ++j) {
            alloc_buf[j] = random() % 0xFF;
            alloc_buf[j + (memory_size / 2)] = alloc_buf[j];
        }
        sleep(200);
  
        sprintf(buf, "|Address: [0x%Q] Size: [0x%Q] verify data...                      ",
                    alloc_buf, memory_size);
        printat(10, y, buf);

        for (int j = 1; j < memory_size / 2; ++j) {
            if (alloc_buf[j] != alloc_buf[j + (memory_size / 2)]) {
                // sprintf(buf, "|Address: [0x%Q] Size: [0x%Q] failed at %x, %x != %x      ", alloc_buf, memory_size, j, alloc_buf[j], alloc_buf[j + (memory_size / 2)]);
                // printat(10, y, buf);
                printf("Task ID[0x%Q] verify failed %x != %x\n", task->link.id, alloc_buf[j], alloc_buf[j + (memory_size / 2)]);
                exit_task();
            }
        }
        sprintf(buf, "|Address: [0x%Q] Size: [0x%Q] verification success          ", alloc_buf, memory_size);
        printat(10, y, buf);
        free_memory(alloc_buf);
        sleep(200);
    }
    exit_task();
}
static void cmd_testranalloc(const char *param) {
    for (int i = 0; i < 1000; ++i) {
        create_task(TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD, 0, 0, (QWORD)random_alloc_task);
    }
}

static void cmd_hddinfo(const char *param) {
    HDDINFORMATION hdd;
    char buf[100];

    if (read_hdd_information(TRUE, TRUE, &hdd) == FALSE) {
        printf("Failed to read hdd information\n");
        return;
    }

    printf("========== Primary Master HDD Information ==========\n");

    memcpy(buf, hdd.model_number, sizeof(hdd.model_number));
    buf[sizeof(hdd.model_number) - 1] = '\0';
    printf("Model number:\t %s\n", buf);

    memcpy(buf, hdd.serial_number, sizeof(hdd.serial_number));
    buf[sizeof(hdd.serial_number) - 1] = '\0';
    printf("Serial number: \t %s\n", buf);

    printf("Head count:\t %d\n", hdd.number_of_head);
    printf("Cylinder count:\t %d\n", hdd.number_of_cylinder);
    printf("Sector count:\t %d\n", hdd.number_of_sector_per_cylinder);

    printf("Total sector:\t %d Sector, %dMB\n", hdd.total_sectors, hdd.total_sectors / 2 / 1024);
}
static void cmd_readsector(const char *param) {
    struct parameter_list param_list;
    char LBA_param[50], sector_count_param[50];
    DWORD LBA;
    int sector_count;
    BYTE *buf;
    BYTE data;
    BOOL is_exit = FALSE;

    initialize_parameter(&param_list, param);
    if ((get_next_param(&param_list, LBA_param) == 0) ||
        (get_next_param(&param_list, sector_count_param) == 0)) {
        printf("ex) readsector 0(LBA) 10(count)\n");
        return;
    }
    LBA = atoi(LBA_param, 10);
    sector_count = atoi(sector_count_param, 10);

    buf = allocate_memory(sector_count * 512);
    if (read_hdd_sector(TRUE, TRUE, LBA, sector_count, buf) == sector_count) {
        printf("LBA [%d], [%d] sector read success!!", LBA, sector_count);
        for (int i = 0; i < sector_count; ++i) {
            for (int j = 0; j < 512; ++j) {
                if (!((i == 0) && (j == 0)) && ((j % 256) == 0)) {
                    printf("\nPress any key to continue... ('q' is exit): ");
                    if (getch() == 'q') {
                        is_exit = TRUE;
                        break;
                    }
                }
                if ((j % 16) == 0) {
                    printf("\n[LBA:%d, Offset:%d]\t| ", LBA + i, j);
                }

                data = buf[i * 512 + j] & 0xFF;
                if (data < 16) {
                    printf("0");
                }
                printf("%X ", data);
            }

            if (is_exit == TRUE) {
                break;
            }
        }
        printf("\n");
    }
    else {
        printf("Read Failed!\n");
    }

    free_memory(buf);
}
static void cmd_writesector(const char *param) {
    struct parameter_list param_list;
    char LBA_param[50], sector_count_param[50];
    DWORD LBA;
    int sector_count;
    BYTE *buf;
    BOOL is_exit = FALSE;
    BYTE data;
    static DWORD g_write_count = 0;

    initialize_parameter(&param_list, param);
    if ((get_next_param(&param_list, LBA_param) == 0) ||
        (get_next_param(&param_list, sector_count_param) == 0)) {
        printf("ex) writesector 0(LBA) 10(count)\n");
        return;
    }
    
    LBA = atoi(LBA_param, 10);
    sector_count = atoi(sector_count_param, 10);

    g_write_count++;

    buf = allocate_memory(sector_count * 512);

    for (int i = 0; i < sector_count; ++i) {
        for(int j = 0; j < 512; j += 8) {
            *(DWORD *)&(buf[i * 512 + j]) = LBA + i;
            *(DWORD *)&(buf[i * 512 + j + 4]) = g_write_count;
        }
    }

    if (write_hdd_sector(TRUE, TRUE, LBA, sector_count, buf) != sector_count) {
        printf("Write fail\n");
        return;
    }

    printf("LBA [%d], [%d] Sector read success!!", LBA, sector_count);
    
    for (int i = 0; i < sector_count; ++i) {
        for (int j = 0; j < 512; ++j) {
            if (!((i == 0) && (j == 0)) && ((j % 256) == 0)) {
                printf("\nPress any key to continue... ('q' is exit) : ");
                if (getch() == 'q') {
                    is_exit = TRUE;
                    break;
                }
            }
            if ((j % 16) == 0) {
                printf("\n[LBA:%d, Offset:%d]\t| ", LBA + i, j);
            }
            data = buf[i * 512 + j] & 0xFF;
            if (data < 16) {
                printf("0");
            }
            printf("%X ", data);
        }
        if (is_exit == TRUE) {
            break;
        }
    }
    printf("\n");
    free_memory(buf);
}

static void cmd_testsleep(const char *param) {
    BYTE second_before, minute_before, hour_before;
    BYTE second_after, minute_after, hour_after;
    printf("Testing 10 seconds sleep...\n");
    read_rtc_time(&hour_before, &minute_before, &second_before);
    sleep(10000);
    read_rtc_time(&hour_after, &minute_after, &second_after);

    printf("[Sleep(10000)] Elapsed %d seconds\n\n", ((hour_after - hour_before) * 3600) +
                                                    ((minute_after - minute_before) * 60) +
                                                    (second_after - second_before));
    
    printf("Testing 10000 1ms sleep\n");
    read_rtc_time(&hour_before, &minute_before, &second_before);
    for (int i = 0; i < 10000; ++i) {
        sleep(1);
    }
    read_rtc_time(&hour_after, &minute_after, &second_after);
    printf("[10000*Sleep(1)] Elapsed %d seconds\n\n", ((hour_after - hour_before) * 3600) +
                                                        ((minute_after - minute_before) * 60) +
                                                        (second_after - second_before));
}

static void cmd_mounthdd(const char *param) {
    if (mount() == FALSE) {
        printf("HDD mount failed\n");
        return;
    }
    printf("HDD mount success\n");
}

static void cmd_formathdd(const char *param) {
    if (format() == FALSE) {
        printf("HDD format failed\n");
        return;
    }
    printf("HDD format success\n");
}

static void cmd_filesysteminfo(const char *param) {
    FILESYSTEMMANAGER fs_manager;
    get_file_system_information(&fs_manager);

    printf("========== File System Information ==========\n");
    printf("Mounted                         : %d\n", fs_manager.is_mounted);
    printf("Reserved Sector count           : %d Sector\n", fs_manager.reserved_sector_count);
    printf("Cluster Link Table Start Address: %d Sector\n", fs_manager.cluster_link_area_start_address);
    printf("Cluster Link Table Size         : %d Sector\n", fs_manager.cluster_link_area_size);
    printf("Data Area Start Address         : %d Sector\n", fs_manager.data_area_start_address);
    printf("Total Cluster Count             : %d Cluster\n", fs_manager.total_cluster_count);
}

static void cmd_createfile(const char *param) {
    struct parameter_list param_list;
    char filename[50];
    unsigned int length;
    DWORD cluster;
    DIRECTORYENTRY entry;
    DWORD free_entry;

    initialize_parameter(&param_list, param);
    length = get_next_param(&param_list, filename);
    filename[length] = '\0';
    if (length > (sizeof(entry.file_name)) - 1) {
        printf("File name too long!\n");
        return;
    }
    else if (length == 0) {
        printf("File name cannot be NULL\n");
        return;
    }

    cluster = find_free_cluster();
    if ((cluster == FILESYSTEM_LASTCLUSTER) ||
        (set_cluster_link_data(cluster, FILESYSTEM_LASTCLUSTER) == FALSE)) {
        printf("Failed to allocate cluster\n");
        return;
    }

    free_entry = find_free_directory_entry();
    if (free_entry == -1) {
        set_cluster_link_data(cluster, FILESYSTEM_FREECLUSTER);
        printf("Directory entry is full\n");
        return;
    }

    memcpy(entry.file_name, filename, length + 1);
    entry.start_cluster_index = cluster;
    entry.file_size = 0;

    if (set_directory_entry_data(free_entry, &entry) == FALSE) {
        set_cluster_link_data(cluster, FILESYSTEM_FREECLUSTER);
        printf("Failed to set directory entry\n");
    }

    printf("File created successfully\n");
}

static void cmd_deletefile(const char *param) {
    struct parameter_list param_list;
    char filename[50];
    unsigned int length;
    DIRECTORYENTRY entry;
    int offset;

    initialize_parameter(&param_list, param);
    length = get_next_param(&param_list, filename);
    filename[length] = '\0';

    if (length > (sizeof(entry.file_name) - 1)) {
        printf("File name too long!\n");
        return;
    }
    else if (length == 0) {
        printf("File name cannot be NULL\n");
        return;
    }

    offset = find_directory_entry(filename, &entry);
    if (offset == -1) {
        printf("File not found : %s\n", filename);
        return;
    }

    if (set_cluster_link_data(entry.start_cluster_index, FILESYSTEM_FREECLUSTER) == FALSE) {
        printf("Failed to free cluster\n");
        return;
    }

    memset(&entry, 0, sizeof(entry));
    if (set_directory_entry_data(offset, &entry) == FALSE) {
        printf("Failed to update root directory\n");
    }

    printf("File deleted successfully\n");
}

static void cmd_dir(const char *param) {
    BYTE *cluster_buffer;
    int count, total_count;
    DIRECTORYENTRY *entry;
    char buf[400];
    char temp_value[50];
    DWORD total_byte;

    cluster_buffer = allocate_memory(FILESYSTEM_SECTORSPERCLUSTER * 512);

    if (read_cluster(0, cluster_buffer) == FALSE) {
        printf("Failed to read root directory\n");
        return;
    }

    entry = (DIRECTORYENTRY *)cluster_buffer;
    total_count = 0;
    total_byte = 0;
    for (int i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; ++i) {
        if (entry[i].start_cluster_index == 0) {
            continue;
        }
        total_count++;
        total_byte += entry[i].file_size;
    }

    entry = (DIRECTORYENTRY *)cluster_buffer;
    count = 0;

    for (int i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; ++i) {
        if (entry[i].start_cluster_index == 0) {
            continue;
        }

        memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        memcpy(buf, entry[i].file_name, strlen(entry[i].file_name));

        sprintf(temp_value, "%d bytes", entry[i].file_size);
        memcpy(buf + 30, temp_value, strlen(temp_value));

        sprintf(temp_value, "0x%X cluters", entry[i].start_cluster_index);
        memcpy(buf + 55, temp_value, strlen(temp_value) + 1);
        printf("    %s\n", buf);

        if ((count != 0) && ((count % 20) == 0)) {
            printf("Press any key to continue... ('q' is exit) : ");
            if (getch() == 'q') {
                printf("\n");
                break;
            }
        }

        count++;
    }

    printf("\t Total file count: %d\t Total File Size: %d bytes\n", total_count, total_byte);
    free_memory(cluster_buffer);
}
