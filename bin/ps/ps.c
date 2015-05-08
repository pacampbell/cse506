#include <stdlib.h>
#include <stdio.h>
#define NUM 8096
int main(int argc, char *argv[]) {
	// ps();
    char c[NUM];
    for(int i = 0; i < NUM; i++) {
        c[i] = 'A' + (i % 26);
        printf("%d %c %p\n", i, c[i], (c + i));
    }
    // printf("char: %c\n", *c);
	return EXIT_SUCCESS;
}
