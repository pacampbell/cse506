#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

int main(int argc, char *argv[], char* envp[]) {
    //char buffer[INPUT_BUFFER];
    int running = 1;
    char *test = "Hello, World!\n";
    size_t len = strlen(test);
    while(running && 0) {
        //printf("> ");
        //scanf("%"XSTR(INPUT_BUFFER)"s", buffer);
    }

    write(1, test, len);
    return 1337;
}
