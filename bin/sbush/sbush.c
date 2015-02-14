#include "sbush.h"

int test_main(int argc, char *argv[], char* envp[]);
int sbsh_main(int argc, char* argv[], char* envp[]);
int pipe_count(char *cmd);
int count_pipes(char *cmd);

int main(int argc, char *argv[], char* envp[]) {
    // return test_main(argc, argv, envp);
    return sbsh_main(argc, argv, envp);
}

int run_cmd(char ***cmds, char* envp[]) {
    char *c, *path;
    int pipe_count = 0;

    for(pipe_count = 0; *(cmds+pipe_count) != NULL; pipe_count++);
    pipe_count--;

    int fds[pipe_count][2];

    for(int i = 0; i < pipe_count; i++) {
        if( pipe(*(fds + i)) != 0 ) {
            printf("Error: pipe\n");
            return -1;
        }
    }

    for(int i = 0; cmds[i] != NULL; i++) {

        if(fork() == 0) {
            //child

            if(i == 0 && cmds[i+1] == NULL) {
                goto one_cmd;
            } else if(i == 0) {
                //if first cmd
                if(dup2(fds[0][1], 1) < 0) {
                    printf("dup2 broke 1\n");
                } 

            } else if(cmds[i+1] == NULL) {
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
            path = find_env_var(envp, "PATH");

            //printf("path: %s\n", path);
            c = strtok(path, ':');
            while(c != NULL) {
                execve(strappend(c, "/", *cmds[i]), cmds[i], envp);
                c = strtok(NULL, ':');
            }

            write(STDERR_FILENO, "broke\n", 6);
            exit(1);
        } 
    }

    //while(waitpid(-1, &pipe_count, 0) > 0);

    return 13;
}


int sbsh_main(int argc, char* argv[], char* envp[]) {
    char cmd[256] = {0};
    char *ps1;
    int rc = 0;
    int running = 1;
    int red = 0;
    //char *c;

    while(running) {

        ps1 = find_env_var(envp, "PS1");

        /*for(c = *envp, rc = 0; c != NULL; rc++, c = *(envp + rc)) {
          printf("ENV::: %s\n", c);
          }*/

        if(ps1 == NULL) {
            printf("> ");
            red = read(STDIN_FILENO, cmd, 256);
            *(cmd + red) = '\0';
        } else {
            printf("%s", ps1);
        }

        if(strcmp(cmd, "exit\n") == 0) {
            exit(0);
        }

        char ***commands = extract_commands(cmd);
    
        //check for cd
        if(commands[0] != NULL && strcmp(commands[0][0], "cd") == 0) {
            chdir(commands[0][1]);
            //execve(const char *filename, char *const argv[], char *const envp[]) {
            //printf("exec: %d\n", execve(argv[0], argv, envp));
            //printf("cd broke\n");
            //exit(1);
            continue;
        }


        run_cmd(commands, envp);
        while(waitpid(-1, &rc, 0) > 0) {
            //printf("rc: %d\n", rc);
            if(rc == 0) break;
        }
    }

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

