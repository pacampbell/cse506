#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

#define MAX_ARGS 1

int main(int argc, char *argv[], char* envp[]) {
    int running = 1;
    char buf[] = "hello everyone";

    while(running) {
        printf("> ");
        scanf("%s", buf);
        printf("new buf: %s end", buf);
        running = 0;
    }

    return 13;
}

int new_main(int argc, char* argv[], char* envp[]) {
    char cmd[256] = {0};
    char *name[] = {"fake", NULL};
    char *path;
    int rc;
    char *c;

    //char *str = malloc(8);
    //str[0] = '\0';

    write(STDOUT_FILENO, "> ", 2);
    read(STDIN_FILENO, cmd, 256);

    c = cmd;
    for(;*c != '\n' && *c != 0; c++);
    *c = 0;

    //find the path var
    for(rc = 0, path = *envp; !strbegwith("PATH", path); rc++, path = *(envp+rc));
    for(;*path != '='; path++);
    path++;

    //printf("path: %s\n", path);

    rc = execve(cmd, name, envp);
    c = strtok(path, ':');
    while(c != NULL) {
        //printf("trying: %s\n", strappend(c, "/", cmd));
        rc = execve(strappend(c, "/", cmd), name, envp);
        c = strtok(NULL, ':');
    }

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
