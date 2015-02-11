#include <stdlib.h>
#include <syscall.h>
#include <string.h>
#include <sys/mman.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

/* Head pointer for malloc/free implementation */
static void *head_ptr = NULL;

void *malloc(size_t size) {
    void *ptr = NULL;
    if(head_ptr == NULL) {
        // fd and offset are ignored, fd should be -1 for some implementations
        head_ptr = mmap(NULL, _SC_PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        // Check the result
        if(head_ptr == MAP_FAILED) {
            return NULL;
        }
        // Create a new head struct
        struct page_data pd = { .allocations = 0, .next = NULL };
        memcpy(head_ptr, &pd, PAGE_DATA_SIZE);
    }
    // Find a place for the new allocations
    struct page_data *pg_hd = (struct page_data*)head_ptr;
    // Make the stub for the new allocation
    struct meta_data md = { .size = size, .in_use = 1, .next = NULL};
    // Now determine where to put it
    if(pg_hd->allocations == 0) {
        ptr = pg_hd + PAGE_DATA_SIZE;
        memcpy(ptr, &md, META_DATA_SIZE);
        // Move the pointer up to where the user is allowed to access the memory
        ptr += META_DATA_SIZE;
    } else {
        // Get the first node
        struct meta_data *node = (struct meta_data *)(pg_hd + PAGE_DATA_SIZE);
        if(node->in_use) {
            // TODO: Start here
        }
    }
    return ptr;
}

void free(void *ptr) {
    // TODO:
}

void *sbrk(uint64_t bytes) {
    char *cur = (char*)syscall_1(SYS_brk, 0);
    char *tmp = cur + bytes;
    char *now = (char*)syscall_1(SYS_brk, (uint64_t)tmp);
    return (void*)now;

}

int brk(void *end_data_segment) {
    return (void*)syscall_0(SYS_brk) == end_data_segment;
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
    //TODO: use malloc and fix everything
    struct DIR dir;

    //check if malloc worked

    dir._DIR_fd = open(name, O_RDONLY|O_DIRECTORY);
    if(dir._DIR_fd < 0) {
        //TODO: free dir
        return NULL;
    }

    dir._DIR_avail = 0;
    dir._DIR_next  = NULL;

    return NULL;
}

struct dirent *readdir(struct DIR *dir) {
    struct dirent *ret;

    if ( !dir->_DIR_avail ) {
        //TODO: not sure what to do here
        return NULL;
    }

    ret = dir->_DIR_next;

    dir->_DIR_next   = (struct dirent*)((char*)ret+ ret->d_reclen);
    dir->_DIR_avail -= ret->d_reclen;

    return ret;

}

//int closedir(struct DIR *dir) {
int closedir(struct DIR *dir) {
    //TODO: use free

    return close(dir->_DIR_fd);
}
