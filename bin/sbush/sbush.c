#include <stdio.h>
#include <stdlib.h>
#include "sbush.h"

int main(int argc, char *argv[], char* envp[]) {
    char buffer[INPUT_BUFFER];
    int running = 1;
    while(running) {
        printf("> ");
        scanf("%"XSTR(INPUT_BUFFER)"s", buffer);
    }
    return EXIT_SUCCESS;
}
