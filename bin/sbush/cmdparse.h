#ifndef _CMD_PARSE_H
#define _CMD_PARSE_H
#include <stdlib.h>
#include <string.h>

char*** extract_commands(const char *cmd_str, char *envp[]);

#endif
