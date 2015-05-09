#include <stdlib.h>
#include <stdio.h>
// #define NUM 8192
// #define NUM 4
#define NUM 4096

void test_f(void * addr);

int main(int argc, char *argv[]) {
    ps();
    printf("DID I EVEN GET HERE?\n");
    char test[NUM];
    for(int i = 0; i < NUM; i++) {
       test[i] = 'A' + (i % 26);
        printf("Test: %d - %c\n", i, test[i]);
    }
    printf("Test: %d - %c\n", NUM - 1, test[NUM - 1]);

    // printf("BEFORE CALL TO TEST\n");
    // test_f(test);
    // printf("AFTER CALL TO TEST\n");

    // printf("0x%x 0x%x 0x%x\n", test, test + NUM / 2, test + NUM - 1);

    return EXIT_SUCCESS;
}

void test_f(void * addr) {
    // do nothing
    printf("========= 0x%x\n", addr);
}


