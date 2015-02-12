#ifndef _SBUSH_H
#define _SBUSH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cmdparse.h"

#define MAX_ARGS 1

char *find_env_var(char* envp[], char* name);

// Stringification - https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define XSTR(A) STR(A)
#define STR(A) #A
#define INPUT_BUFFER 1024

#endif
