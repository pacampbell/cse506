#include <stdlib.h>
#include <syscall.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

void putint(int value) {
    // syscall_3()
    // syscall_3(SYS_write, 1, "Test\n", 5);
    syscall_3(SYS_write, 1, 1, 1);
}
