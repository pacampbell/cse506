#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char c[20];
    printf("Enter a file, no more than 19 chars: ");
    read(0, c, 20);
    printf("GOT: %s\n", c);
    int i = open(c, O_RDONLY);

    printf("file no: %d\n", i);
    printf("contents of file\n");

    while(read(i, c, 5) > 0) {

        //c[6] = '\0';
        printf("%s", c);
    }
    printf("\n");
    int rtn = close(i);
    printf("close: %d\n", rtn);

    return 0;
}
