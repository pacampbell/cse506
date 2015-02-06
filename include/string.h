#ifndef _STRING_H
#define _STRING_H
#include <sys/defs.h>
typedef uint64_t size_t;

char *strcpy(char*d,const char*s);
size_t strlen(const char*s);
int strcmp(const char *s1, const char *s2);

#endif
