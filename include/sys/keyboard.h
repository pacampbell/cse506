#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include <sys/defs.h>
#include <sys/common.h>
void init_keyboard(void);
int gets(uint64_t addr, size_t len);
#endif
