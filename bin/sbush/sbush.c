#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sbush.h"

#define MAX_ARGS 1

char *find_env_var(char* envp[], char* name);

int test_main(int argc, char *argv[], char* envp[]);
int sbsh_main(int argc, char* argv[], char* envp[]);
int pipe_count(char *cmd);
int count_pipes(char *cmd);

int main(int argc, char *argv[], char* envp[]) {
    return test_main(argc, argv, envp); 
}

int test_main(int argc, char *argv[], char* envp[]) {
    //char *cmd[] = {"/usr/bin/ls", "/usr/bin/cat", "/usr/bin/cat", NULL};
    char *cmd[] = {"/usr/bin/ls", "/usr/bin/cat", NULL};
    char *fake[][2] = {{"ls", NULL}, {"cat", NULL}};
    //int pipe_count = 2;
    int pipe_count = 1;
    int fds[pipe_count][2];

    for(int i = 0; i < pipe_count; i++) {
        if( pipe(*(fds + i)) != 0 ) {
            printf("Error: pipe\n");
            return -1;
        }
    }

    printf("pipes: %d\n", pipe_count);

    for(int i = 0; *(cmd + i) != NULL; i++) {
        //if only one cmd just do it
        if(i == 0 && *(cmd + i + 1) == NULL) {
            printf("going to one_cmd\n");
            goto one_cmd;
        }

        if(fork() == 0) {
            //child

            if(i == 0) {
                //if first cmd
                if(dup2(fds[0][1], 1) < 0) {
                    printf("dup2 broke 1\n");
                } 

            } else if(*(cmd + i + 1) == NULL) {
                //if last cmd
                if(0 > dup2(fds[i-1][0], 0)) {
                    printf("dup2 broke 2\n");
                }

            } else {
                //if mid cmd
                if(0 > dup2(fds[i-1][0], 0)) {
                    printf("dup2 broke 3\n");
                }

                if(0 > dup2(fds[i][1], 1)) {
                    printf("dup2 broke 4\n");
                }

            }
            
            //close all fds
            for(int j = 0; j < pipe_count; j++) {
                close(fds[j][0]);
                close(fds[j][1]);
            }
one_cmd:
            execve(cmd[i], fake[i], envp);
            exit(1);
        } 
    }

    while(waitpid(-1, &pipe_count, 0) > 0);

    return 13;
}


int sbsh_main(int argc, char* argv[], char* envp[]) {
    char cmd[256] = {0};
    char *name[] = {"fake", NULL};
    char *path;
    char *ps1;
    int rc;
    char *c;

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


int count_pipes(char *cmd) {
    char *cp;
    int count = 0;

    for(cp = cmd; *cp != '\n' && *cp != '\0'; cp++) {
        if(*cp == '|') {
            count++;
        }
    }
    return count;
}

