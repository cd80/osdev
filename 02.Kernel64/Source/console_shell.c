#include "console_shell.h"
#include "console.h"
#include "keyboard.h"
#include "utility.h"

struct shell_command_entry command_table[] = {
    { "help", "Show help", cmd_help },
    { "cls", "Clear screen", cmd_cls },
    { "totalram", "Show total RAM size", cmd_totalram },
    { "strtod", "Convert string to decimal/hex", cmd_strtod },
    { "reboot", "Reboot OS", cmd_reboot },
};

void start_console_shell(void) {
    // Main loop of the shell
    char cmd_buf[CONSOLESHELL_MAXCMDBUFCOUNT];
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
            (memcmp(command_table[i].cmd, cmd_buf, space_idx) == 0)) {
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

    memcpy(param, list->buf + list->cur_pos, i);
    length = i - list->cur_pos;
    param[length] = '\0';

    list->cur_pos += length + 1;
    return length;
}

void cmd_help(const char *param) {
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
    }
}

void cmd_cls(const char *param) {
    clear_screen();
    set_curosr(0, 1);
}

void cmd_totalram(const char *param) {
    printf("Total RAM size = %d MB\n", get_total_ram_size());
}

void cmd_strtod(const char *param_buf) {
    char param[100];
    int length;
    struct parameter_list param_list;
    int count = 0;
    long value;

    initialize_parameter(&param_list, param_buf);

    while (1) {
        length = get_next_param(&param_list, param);
        if (length == 0) {
            break;
        }

        printf("param %d = '%s', length = %d, ", count + 1, param, length);

        if (memcmp(param, "0x", 2) == 0) {
            value = atoi(param + 2, 16);
            printf("HEX Value = %q\n", value);
        }
        else {
            value = atoi(param, 10);
            printf("Decimal Value = %d\n", value);
        }

        count++;
    }
}

void cmd_reboot(const char *param) {
    printf("[*] System reboot\n");
    printf("press any key to restart...");
    getch();
    reboot();
}
