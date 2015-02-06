#include <stdlib.h>
#include <syscall.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return syscall_3(SYS_write, fd, (uint64_t)buf, count);
}
