#include <stdlib.h>
#include <syscall.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

void exit(int status) {
    syscall_1(SYS_exit, status);
}

/* Head pointer for malloc/free implementation */
static void *head_ptr = NULL;

void *malloc(size_t size) {
    void *ptr = NULL;
    if(size > 0) {
        // Calculate the total size of the allocation
        size_t total_bytes = size + META_DATA_SIZE;
        // Align on word boundry
        total_bytes += total_bytes % 8;
        // Allocate a meta_data struct
        struct meta_data md = {.size = total_bytes, .in_use = 1, .next = NULL,
            .prev = NULL, .data_start = NULL};
        // Check the head pointer
        if(head_ptr == NULL) {
            // Get back the address in memory
    printf("HELLO###################################################\n");
            ptr = sbrk(total_bytes); // Get the current brk
    printf("HELLO###################################################\n");
            // Linux system call returns the new brk
            if(ptr == (void*) -1) {
                // sbrk failed
                return NULL;
            }
            // Set the data_start correctly
            md.data_start = ptr + META_DATA_SIZE;
            // Copy the struct into the block
            memcpy(ptr, &md, META_DATA_SIZE);
            // Set the head pointer
            head_ptr = ptr;
            // Finally adjust the address for the user
            ptr += META_DATA_SIZE;
        } else {
            // Navigate the list
            struct meta_data *node = head_ptr;
            struct meta_data *prev = NULL;
            int placed = 0;
            while(node != NULL) {
                if(node->in_use || (!(node->in_use) && node->size < total_bytes)) {
                    prev = node;
                    node = node->next;
                } else {
                    // Found a place for the allocation
                    placed = 1;
                    // Change the metadata struct
                    node->in_use = 1;
                    // Set the pointer to the start of the data
                    ptr = node->data_start;
                    // Done Allocating memory
                    break;
                }
            }
            // Check to see if we found any successful nodes
            if(!placed) {
                // Allocate space for a new block
                ptr = sbrk(total_bytes);
                if(ptr == (void*) -1) {
                    // sbrk failed
                    return NULL;
                }
                // Set the data_start correctly
                md.data_start = ptr + META_DATA_SIZE;
                // Add this new node to the end of the list
                md.prev = prev;
                prev->next = ptr;
                // Copy the struct into the block
                memcpy(ptr, &md, META_DATA_SIZE);
                // Finally adjust the address for the user
                ptr += META_DATA_SIZE;
            }
        }
    }
    return ptr;
}

void free(void *ptr) {
    if(ptr != NULL) {
        // Begin searching for the ptr
        struct meta_data *node = head_ptr;
        int freed_block = 0;
        while(node != NULL) {
            if(node->data_start == ptr) {
                // Mark the block free for use
                node->in_use = 0;
                // Mark that we freed the block
                break;
            } else {
                node = node->next;
            }
        }
        if(!freed_block) {
            // If we get here this is technically an error
            // user tried to free memory they shouldn't have
        }
    }
}

void *sbrk(uint64_t bytes) {
    char *cur = (char*) syscall_1(SYS_brk, 0);
    char *tmp = cur + bytes;
    syscall_1(SYS_brk, (uint64_t)tmp);
    return (void*)cur;
}

int brk(void *end_data_segment) {
    // TODO: Does this work?
    return (void*) syscall_1(SYS_brk, 0) == end_data_segment;
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
    return syscall_2(SYS_dup2, oldfd, newfd);
}

void *opendir(const char *name) {
    struct DIR *dir = malloc(sizeof(sizeof(struct DIR)));
    if(dir == NULL) {
        return NULL;
    }

    dir->_DIR_fd = open(name, O_RDONLY|O_DIRECTORY);
    if(dir->_DIR_fd < 0) {
        //TODO: free dir
        return NULL;
    }

    dir->_DIR_avail = 0;
    dir->_DIR_next  = NULL;

    return dir;
}

struct dirent *readdir(struct DIR *dir) {
    //not thread safe
    static struct dirent *ret;

    if ( !dir->_DIR_avail ) {
        //TODO: not sure what to do here
        return NULL;
    }

    ret = dir->_DIR_next;

    dir->_DIR_next   = (struct dirent*)((char*)ret+ ret->d_reclen);
    dir->_DIR_avail -= ret->d_reclen;

    return ret;

}

void ps() {
    syscall_0(SYS_ps);
}

void yield() {
    syscall_0(SYS_yield);
}

//int closedir(struct DIR *dir) {
int closedir(struct DIR *dir) {
    free(dir);
    return close(dir->_DIR_fd);
}

char *find_env_var(char* envp[], char* name) {
    int rc;
    char *var;

    for(rc = 0, var = *envp; var != NULL && !strbegwith(name, var); rc++, var = *(envp+rc));
    if(var != NULL) {
        for(; *var != '='; var++);
        var++;
    }

    return var;
}

int atoi(char *cp) {
    int rtn = 0;

    for(; *cp != '\0'; cp++) {
        if((int)*cp < 0x30 || 0x39 < (int)*cp) {
            return -99;
        }

        rtn *= 10;
        rtn += *cp - 0x30;
    }

    return rtn;
}

//this one gives you the original pointer
int find_env_var_orig(char* envp[], char* name) {
    int rc;

    for(rc = 0; envp[rc] != NULL && !strbegwith(name, envp[rc]); rc++);
    return rc;
}

int kill(pid_t pid) {
    return syscall_1(SYS_kill, pid);
}

