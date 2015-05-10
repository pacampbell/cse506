#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int i = open("bin/hello", 0);
    printf("file no: %d\n", i);
    return 0;
}
