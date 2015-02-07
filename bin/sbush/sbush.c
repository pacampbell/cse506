#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

#define MAX_ARGS 1

int old_main(int argc, char *argv[], char* envp[]) {
    //char buffer[INPUT_BUFFER];
    int running = 1;
    char *test = "Hello, World!\n";
    char *test2 = "enter something: ";
    char *test3 = "got: ";
    char *nl = "\n";

    char buf[5] = {0};

    size_t len = strlen(test);
    while(running && 0) {
        //printf("> ");
        //scanf("%"XSTR(INPUT_BUFFER)"s", buffer);
        running = 0;
    }
    putint(argc);
    write(STDOUT_FILENO, argv[0], strlen(argv[0]));
    write(STDOUT_FILENO, nl, 1);
    write(STDOUT_FILENO, envp[0], strlen(envp[0]));
    write(STDOUT_FILENO, nl, 1);
    write(STDOUT_FILENO, test, len);
    write(STDOUT_FILENO, test2, 17);
    // read(STDIN_FILENO, buf, 5);
    write(STDOUT_FILENO, test3, 5);
    write(STDOUT_FILENO, buf, 5);
    write(STDOUT_FILENO, "\n", 1);

    int fd = open("./test", O_WRONLY);

    if(fd > 0) {
        write(STDOUT_FILENO, "good fd\n", 8);
        write(fd, "good fd\n", 8);
        if(close(fd) == 0) {
            write(STDOUT_FILENO, "good close\n", 11);
        } else {
            write(STDOUT_FILENO, "bad close\n", 10);
        }

    } else {
        write(STDOUT_FILENO, "bad fd\n", 7);
    }

    printf("\nnum: ", 1337);

    return 13;
}

int main(int argc, char* argv[], char* envp[]) {
    char buf[256] = {0};
    char *name[] = {"fake", NULL};
    int rc;
    char *c;

    write(STDOUT_FILENO, "> ", 2);
    read(STDIN_FILENO, buf, 256);

    c = buf;
    for(;*c != '\n' && *c != 0; c++);
    *c = 0;

    rc = execve(buf, name, NULL/*char *const envp[]*/);
    write(STDOUT_FILENO, "no\n", 3);

    return rc;
}

/*
void evaluateCommand(char **cmd, int cmdSize, int *running, char* wd, char** envp, int debug, char *historyList[], int rdSize) {
    char *arguments[MAX_ARGS];

    // Something went wrong stop evaluating.
    if(cmdSize <= 0){
        return;
    }
    // Check to see how many commands we need to evaluate
    if(!rdSize){
        if (strlen(*cmd)) {
            if (debug) {
                printf("RUNNING:%s\n", *cmd);
            }
            parseCommand(*cmd, arguments, MAX_ARGS);
            if (!strcmp(arguments[0], "exit")) {
                *running = false;

            } else if (!strcmp(arguments[0], "cd")) {


                if (arguments[1] == NULL || !strcmp(arguments[1], "~")) {
                    setenv("OLDPWD", wd, 1);
                    if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
                    chdir(parseEnv(envp, "HOME"));

                } else if (!strcmp(arguments[1], "-")) {
                    chdir(getenv("OLDPWD"));
                } else {
                    setenv("OLDPWD", wd, 1);
                    if (debug) printf("oldpwd: %s\n", getenv("OLDPWD"));
                    int val = chdir(arguments[1]);
                    if (val) printf("Sorry but %s does not exist\n", arguments[1]);
                }

            } else if (!strcmp(arguments[0], "set")) {
                setenv(arguments[1], arguments[3], 1);

            } else if (!strcmp(arguments[0], "echo")) {
                int index = contains(arguments, '$');
                char *cp = NULL;
                if(index > -1) cp = arguments[index];
                if (cp != NULL) {
                    cp++;
                    printf("%s\n", getenv(cp));
                } else {
                    spawn(arguments);
                }

            } else if(!strcmp(arguments[0], "history")) {
                for (int i = 0; i < MAX_HISTORY && *historyList[i] != '\0'; ++i) {
                    printf("%s\n", historyList[i]);
                }
            } else if(!strcmp(arguments[0], "wolfie")){
                printWolf();
            } else if(!strcmp(arguments[0], "cls")){
                strcpy(arguments[0], "clear");
                spawn(arguments);
            } else if(!strcmp(arguments[0], "clear")){
                for (int i = 0; i < MAX_HISTORY; ++i) {
                    *historyList[i] = '\0';
                }
            } else if(!strcmp(arguments[0], "clear")){
                for (int i = 0; i < MAX_HISTORY; ++i) {
                    *historyList[i] = '\0';
                }
            } else {
                spawn(arguments);
            }
            if (debug) {
                printf("ENDED: %s (needs return val)\n", *cmd);
            }
        }
    } else {
        spawnRedirect(cmd, cmdSize, redirects, rdSize);
    }

}*/
