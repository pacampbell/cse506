#ifndef _STRING_H
#define _STRING_H

#ifndef _SIZE_T
#define _SIZE_T
    typedef unsigned long size_t;
#endif

char *strcpy(char*d,const char*s);
size_t strlen(const char*s);
int strcmp(const char *s1, const char *s2);
#endif
