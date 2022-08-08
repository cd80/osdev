#ifndef __console_shell_h__
#define __console_shell_h__

#include "types.h"

#define CONSOLESHELL_MAXCMDBUFCOUNT 300
#define CONSOLESHELL_PROMPT         "cd80> "

typedef void (*COMMAND_FUNCTION) (const char *param);

#pragma pack(push, 1)

struct shell_command_entry {
    char *cmd;
    char *help_msg;
    COMMAND_FUNCTION handler;
};

struct parameter_list {
    const char *buf;
    int length;
    int cur_pos;
};

#pragma pack(pop)

void start_console_shell(void);
void execute_command(const char *cmd_buf);
void initialize_parameter(struct parameter_list *list, const char *param);
int get_next_param(struct parameter_list *list, char *param);

void cmd_help(const char *param);
void cmd_cls(const char *param);
void cmd_totalram(const char *param);
void cmd_strtod(const char *param);
void cmd_reboot(const char *param);
void cmd_settimer(const char *param);
void cmd_wait(const char *param);
void cmd_rdtsc(const char *param);
void cmd_cpuspeed(const char *param);
void cmd_date(const char *param);
void cmd_createtask(const char *param);

#endif