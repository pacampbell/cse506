#include <stdio.h>
#include <stdlib.h>

int q_or_something;
//#define NUM 24000

int main(int argc, char *argv[]) {
    char buf[10];
    q_or_something = 9;
    printf("Hello, World!\n");
    read(0, buf, -9);
    printf("buf: %s\n", buf);
    printf("%d\n", q_or_something);
    return EXIT_SUCCESS;
}
