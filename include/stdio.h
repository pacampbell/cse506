#ifndef _STDIO_H
#define _STDIO_H

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int printf(const char *format, ...);
int scanf(const char *format, ...);
int fprintf(int fd, const char *format, ...);
char *fgets(char *s, int size, int fd);
int convert(const char *str, int base);

#endif
