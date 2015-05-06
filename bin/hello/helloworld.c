#include <stdio.h>
#include <stdlib.h>

int q_or_something;
//#define NUM 24000

int main(int argc, char *argv[]) {
	uint64_t pid = getpid();
    q_or_something = 9;
    printf("Hello, World!\n");
    printf("%d\n", q_or_something);
    printf("pid: %d\n", pid);
    return EXIT_SUCCESS;
}
