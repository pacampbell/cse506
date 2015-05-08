#include <stdio.h>
#include <stdlib.h>

int q_or_something;
//#define NUM 24000

void test();

int main(int argc, char *argv[]) {
	uint64_t pid = getpid();
    char buf[10];
    q_or_something = 9;
    printf("Hello, World!\n");
    // read(0, buf, 9);
    printf("buf: %s\n", buf);
    printf("%d\n", q_or_something);
    printf("pid: %d\n", pid);
    printf("ppid: %d\n", getppid());
    test();
    pid_t cpid;
    switch((cpid = fork())) {
        case 0:
            printf("In child process\n");
            break;
        case -1:
            printf("failed to fork\n");
            break;
        default:
            printf("In parent process. Child pid: %d\n", cpid);
            break;
    }
    printf("Good bye world!\n");
    return EXIT_SUCCESS;
}

void test() {
	printf("In the test function in hello world!\n");
}
