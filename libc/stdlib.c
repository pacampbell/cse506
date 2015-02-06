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

int open(const char *pathname, int flags) {
    return syscall_2(SYS_open, pathname, flags);
}

char *getcwd(char *buf, size_t size) {
    return syscall_2(SYS_getcwd, buf, size);
}

int chdir(const char *path) {
    reteun syscall_1(SYS_chdir, path);
}

off_t lseek(int fildes, off_t offset, int whence) {
    return syscall_3(SYS_lseek, fildes, offset, whence);
}

int close(int fd) {
    return syscall_1(SYS_close, fd);
}

int pipe(int filedes[2]) {
    return syscall_1(SYS_pipe, filedes);
}

int dup(int oldfd) {
    return syscall_1(SYS_dup, oldfd);
}

int dup2(int oldfd, int newfd) {
    return syscall_1(SYS_dup2, oldfd);
}

