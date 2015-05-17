#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    char c[20];
    while(1) {
        printf("Enter a file, no more than 19 chars: ");
        read(0, c, 20);
        printf("execing: %s\n", c);
        execve(c, argv, envp);
        printf("that was a bad program name, try again\n");
    }
}
