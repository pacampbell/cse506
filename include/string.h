#ifndef _STRING_H
#define _STRING_H
#include <sys/defs.h>
#include <stdlib.h>

char *strcpy(char*d,const char*s);
size_t strlen(const char*s);
int strcmp(const char *s1, const char *s2);
int strbegwith(char *str1, char *str2);
void *memset(void *s, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
char *strtok(char *str, const char delim);
char *strappend(char *s1, char *s2, char *s3);
char *strip(char *src);
char **split(const char *str, char delim);
int strcmp(const char* s1, const char* s2);
int count_tokens(const char *cmd, char token);

int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
#endif
