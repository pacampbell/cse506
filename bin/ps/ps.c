#include <stdlib.h>
#include <stdio.h>
#define NUM 8096
int main(int argc, char *argv[]) {
	ps();
        char c[NUM];
        for(int i = 0; i < NUM; i++) {
            c [i] = 'J';
        }

        printf("char: %c\n", *c);
	return EXIT_SUCCESS;
}
