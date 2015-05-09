#include <stdlib.h>
#include <stdio.h>
#define NUM 4000
//#define NUM 4
int main(int argc, char *argv[]) {
    uint64_t stack = 0;

   	// ps();
    {
    char c[NUM];
        
        c[1] = 'A' + (1 % 26);
        printf("%d %c %x\n", 1, c[1], (c + 1));
        __asm__ __volatile__ (
                "movq %%rsp, %0;"
                :"=r"(stack)
                :
                :
                );
        printf("stack: %x\n", stack);}

        __asm__ __volatile__ (
                "movq %%rsp, %0;"
                :"=r"(stack)
                :
                :
                );
        printf("stack: %x\n", stack);


    // printf("char: %c\n", *c);
	return EXIT_SUCCESS;
}
