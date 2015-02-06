#include <stdlib.h>
#include <syscall.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

void *malloc(size_t size) {
    // TODO:
    return (void*) 0;
}

void free(void *ptr) {
        // TODO:
}

int brk(void *end_data_segment) {
    return -1;
}

ssize_t write(int fd, const void *buf, size_t count) {
    return syscall_3(SYS_write, fd, (uint64_t)buf, count);
}

pid_t fork(void) {
    return syscall_0(SYS_fork);
}

pid_t getpid(void) {
    return syscall_0(SYS_getpid);
}

pid_t getppid(void) {
    return syscall_0(SYS_getppid);
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
    return syscall_3(SYS_execve, (uint64_t)filename, (uint64_t) argv, (uint64_t) envp);
}

pid_t waitpid(pid_t pid, int *status, int options) {
    // TODO: Figure out what this does
    return -1;
}

unsigned int sleep(unsigned int seconds) {
    return 1;
}

unsigned int alarm(unsigned int seconds) {
    return syscall_1(SYS_alarm, seconds);
}

char *getcwd(char *buf, size_t size) {
    return (char*) syscall_2(SYS_getcwd, (uint64_t)buf, (uint64_t)size);
}

int chdir(const char *path) {
    return syscall_1(SYS_chdir, (uint64_t)path);
}
