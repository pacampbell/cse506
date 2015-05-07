#include <stdio.h>
#include <stdlib.h>

//#define NUM 1000000000000000000
#define NUM 20000

int main(int argc, char *argv[]) {
    printf("made big array\n");
    char str[NUM];

    for(unsigned long i = 0; i < NUM; i++) {
        /*if(!(i%100))*/printf("looking at index: %d\n", i);
        str[i] = '7';
    pid_t cpid;
    switch((cpid = fork())) {
        case 0:
            printf("child process\n");
            break;
        case -1:
            printf("failed to fork\n");
            break;
        default:
            printf("Child pid: %d\n", cpid);
            break;
    }
    }

    printf("large string: %c\n", str[NUM-1]);
    printf("Hello World!!! \n");
}

