#include <stdlib.h>
#include <syscall.h>
#include <string.h>

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
    // Create the struct
    struct rusage ru;
    // Zero out memory
    memset(&ru, 0, sizeof(ru));
    // According to the man page wait4 with these args is waitpid
    return syscall_4(SYS_wait4, (uint64_t)pid, (uint64_t)status, (uint64_t)options, (uint64_t)&ru);
}

unsigned int sleep(unsigned int seconds) {
    struct timespec rqtp, rmtp;
    // Zero out the memory
    memset(&rqtp, 0, sizeof(rqtp));
    memset(&rmtp, 0, sizeof(rmtp));
    // initialize rqtp struct
    rqtp.tv_sec = seconds;
    // Get the return code
    if(syscall_2(SYS_nanosleep, (uint64_t)&rqtp, (uint64_t)&rmtp) < 0) {
        // TODO: Something bad happened
    }
    return rmtp.tv_sec;
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

ssize_t read(int fd, void *buf, size_t count) {
    return syscall_3(SYS_read, fd, (uint64_t)buf, count);
}

int open(const char *pathname, int flags) {
    return syscall_2(SYS_open, (uint64_t)pathname, flags);
}

off_t lseek(int fildes, off_t offset, int whence) {
    return syscall_3(SYS_lseek, fildes, offset, whence);
}

int close(int fd) {
    return syscall_1(SYS_close, fd);
}

int pipe(int filedes[2]) {
    return syscall_1(SYS_pipe, (uint64_t)filedes);
}

int dup(int oldfd) {
    return syscall_1(SYS_dup, oldfd);
}

int dup2(int oldfd, int newfd) {
    return syscall_1(SYS_dup2, oldfd);
}

void *opendir(const char *name) {
    return NULL;
}

struct dirent *readdir(void *dir) {
    return NULL;
}

int closedir(void *dir) {
    return 1;
}
