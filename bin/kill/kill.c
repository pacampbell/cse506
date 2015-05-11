#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    char pids[5];
    pid_t pid;
    if (argc < 2) {
        printf("Enter a pid: ");
        read(0, pids, 5);
        pid = atoi(pids);
    } else {
        pid = atoi(argv[1]);
    }

    printf("pid: %d\n", (int)pid);
    int rv = kill(pid);
    printf("return: %d\n", rv);

    return rv;
}
