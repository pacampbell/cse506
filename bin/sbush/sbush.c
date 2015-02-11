#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

#define MAX_ARGS 1

char *find_env_var(char* envp[], char* name);

int test_main(int argc, char *argv[], char* envp[]);
int sbsh_main(int argc, char* argv[], char* envp[]);

int main(int argc, char *argv[], char* envp[]) {
    // return test_main(argc, argv, envp);
    return sbsh_main(argc, argv, envp);
}

int test_main(int argc, char *argv[], char* envp[]) {
    void *try = sbrk(2);
    if(try == (void*)-1) {
        printf("error got -1 from sbrk\n");
        exit(-1);
    }
    printf("sbrk: %d\n", try);

    char *cp = (char*)((void*)try);
    *cp = '!';
    *(cp+1) = 0;

    printf("%s\n", cp);

    return 13;
}

int sbsh_main(int argc, char* argv[], char* envp[]) {
    char cmd[256] = {0};
    char *name[] = {"fake", NULL};
    char *path;
    char *ps1;
    int rc;
    char *c;

    char *str = malloc(8);
    char *str2 = malloc(50);
    char *str3 = malloc(80);
    free(str2);
    str2 = malloc(10);
    // Use the strings
    str[0] = 'H';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = '\n';

    str2[0] = 'S';
    str2[1] = '\0';

    str3[0] = 'Q';
    str3[1] = '\0';

    write(STDOUT_FILENO, str, strlen(str));
    write(STDOUT_FILENO, str2, strlen(str2));
    write(STDOUT_FILENO, str3, strlen(str3));

    //char *str = malloc(8);
    //str[0] = '\0';

    ps1 = find_env_var(envp, "PS1");

    for(c = *envp, rc = 0; c != NULL; rc++, c = *(envp + rc)) {
        printf("ENV::: %s\n", c);
    }

    if(ps1 == NULL) {
        printf("> ");
        read(STDIN_FILENO, cmd, 256);
    } else {
        printf("%s", ps1);
    }

    c = cmd;
    for(;*c != '\n' && *c != 0; c++);
    *c = 0;

    //find the path var
    path = find_env_var(envp, "PATH");

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

char *find_env_var(char* envp[], char* name) {
    int rc;
    char *var;

    for(rc = 0, var = *envp; var != NULL && !strbegwith(name, var); rc++, var = *(envp+rc));
    if(var != NULL) {
        for(; *var != '='; var++);
        var++;
    }

    return var;
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
