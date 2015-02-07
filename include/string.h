#ifndef _STRING_H
#define _STRING_H
#include <sys/defs.h>
#include <stdlib.h>

char *strcpy(char*d,const char*s);
size_t strlen(const char*s);
int strcmp(const char *s1, const char *s2);
void *memset(void *s, int c, size_t n);
int printf(const char *format, ...);
#endif
