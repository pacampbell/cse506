#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>
#include <sys/time.h>
#include <stdint.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

extern __thread int errno;

int main(int argc, char* argv[], char* envp[]);
void exit(int status);

// memory
typedef uint64_t size_t;
struct meta_data {
    size_t size;
    int in_use;
    struct meta_data *next;
};

struct page_data {
    size_t allocations;
    struct page_head *next;
};

#define META_DATA_SIZE sizeof(struct meta_data)
#define PAGE_DATA_SIZE sizeof(struct page_data)

void *malloc(size_t size);
void free(void *ptr);
int brk(void *end_data_segment);
int sbrk(uint64_t bytes);

// processes
typedef uint32_t pid_t;
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
int execve(const char *filename, char *const argv[], char *const envp[]);

pid_t waitpid(pid_t pid, int *status, int options);
unsigned int sleep(unsigned int seconds);
unsigned int alarm(unsigned int seconds);

// paths
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

// files
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
typedef int64_t ssize_t;
int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
typedef uint64_t off_t;
off_t lseek(int fildes, off_t offset, int whence);
int close(int fd);
int pipe(int filedes[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);

// directories
#define NAME_MAX 255
struct dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name [NAME_MAX+1];
};

struct DIR {
    int              _DIR_fd;
    size_t           _DIR_avail;
    struct dirent*   _DIR_next;
    struct dirent    _DIR_buff[15];
};
void *opendir(const char *name);
struct dirent *readdir(struct DIR *dir);
int closedir(struct DIR *dir);

#endif
