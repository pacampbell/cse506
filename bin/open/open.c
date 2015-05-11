#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char c[6];
    read(0, c, 5);
    printf("GOT: %s\n", c);
    int i = open("hi", 0);

    printf("file no: %d\n", i);
    printf("contents of file\n");

    while(read(i, c, 5) > 0) {

        //c[6] = '\0';
        printf("%s", c);
    }
    printf("\n");

    return 0;
}
