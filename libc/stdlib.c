#include <stdlib.h>
#include <syscall.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

void putint(int value) {
    char *test = "Test\n";
    // syscall_3()
    // syscall_3(SYS_write, 1, "Test\n", 5);
    syscall_3(SYS_write, 1, (uint64_t)test, 5);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return syscall_3(SYS_write, fd, (uint64_t)buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    return syscall_3(SYS_read, fd, (uint64_t)buf, count);
}


