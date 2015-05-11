#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("execing...\n");
    execve("bin/ps", argv, envp);
    printf("STILL HERE... NOT GOOD!!!\n");
}
