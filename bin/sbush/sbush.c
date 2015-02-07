#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

#define MAX_ARGS 1

int main(int argc, char **argv, char **envp) {
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
    return 4;
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
