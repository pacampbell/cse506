#include "sbush.h"
#include <stdio.h>
#include <sys/mman.h>
#include <env.h>

int test_main(int argc, char *argv[], char* envp[]);
int sbsh_main(int argc, char* argv[]);
int pipe_count(char *cmd);
int count_pipes(char *cmd);
int special_cmds(char ***commands);

char **environ;

int main(int argc, char *argv[], char* envp[]) {
    // return test_main(argc, argv, envp);
char **putenv(char *var, char **envp);
    environ = putenv(NULL, envp);
    return sbsh_main(argc, argv);
}

int run_cmd(char ***cmds) {
    int old_fds[2];
    int new_fds[2];
    for(int i = 0; cmds[i] != NULL; i++) {
        int pid;
        // Create a new pipe if there is more commands
        if(cmds[i + 1] != NULL) {
            pipe(new_fds);
        }
        // Fork a new process
        pid = fork();
        // Figure out if we are in the parent or child
        if(pid == 0) {
            char new_path[2048] = {0};
            char *pathp = find_env_var(environ, "PATH");
            char *c = NULL;

            /* child process */
            if(i > 0) { // There was a command previously in the chain
                dup2(old_fds[0], 0);
                close(old_fds[0]);
                close(old_fds[1]);
            }

            if(cmds[i + 1] != NULL) { // Theres another command in the chain
                close(new_fds[0]);
                dup2(new_fds[1], 1);
                close(new_fds[1]);
            }

            // Begin searching the path to execute binary
            strcpy(new_path, pathp);
            c = strtok(new_path, ':');
            while(c != NULL) {
                char *t = strappend(c, "/", cmds[i][0]);
                execve(t, cmds[i], environ);
                c = strtok(NULL, ':');
            }
            fprintf(STDERR_FILENO, "No such command: %s\n", *cmds[i]);
        } else if(pid < 0) {
            /* something went wrong */
            exit(1);
        } else {
            /* parent process */
            if(i > 0) { // There was a command previously in the chain
                close(old_fds[0]);
                close(old_fds[1]);
            }
            if(cmds[i + 1] != NULL) {
                old_fds[0] = new_fds[0];
                old_fds[1] = new_fds[1];
            }
            // Wait for the process to finish
            waitpid(pid, NULL, 0);
        }
    }
    // Check to see if we ran multiple commands
    if(cmds[0] != NULL && cmds[1] != NULL) {
        close(old_fds[0]);
        close(old_fds[1]);
    }
    return 0;
}


int sbsh_main(int argc, char* argv[]) {
    char cmd[256] = {0};
    char ***commands;
    char *ps1;
    int running = 1;
    int red = 0;
    //char *c;

    if(argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        running = 0;

        if(fd < 0) {
            fprintf(STDERR_FILENO, "error opening script: %s\n", argv[1]);
            fprintf(STDERR_FILENO, "Usage: %s [script]\n", argv[0]);
            exit(1);
        }

        while(fgets(cmd, 256, fd) != NULL) {
            commands = extract_commands(cmd);

            if(special_cmds(commands) > 0) {
                continue;
            }

            run_cmd(commands);
            waitpid(-1, NULL, 0);
        }

        exit(1);
    } else if(argc > 2) {
        fprintf(STDERR_FILENO, "too many args\nUsage: %s [script]\n", argv[0]);
        exit(1);
    }

    while(running) {

        ps1 = find_env_var(environ, "PS1");

        /*for(c = *envp, rc = 0; c != NULL; rc++, c = *(envp + rc)) {
          printf("ENV::: %s\n", c);
          }*/

        if(ps1 == NULL) {
            printf("> ");
        } else {
            printf("%s", ps1);
        }

        red = read(STDIN_FILENO, cmd, 256);
        *(cmd + red) = '\0';

        commands = extract_commands(cmd);

        //check for cd and exit
        if(special_cmds(commands) > 0) continue;

        run_cmd(commands);
        // waitpid(-1, NULL, 0);
    }

    return 0;
}

int special_cmds(char ***commands) {

    if(commands[0] == NULL) {
        return 0;
    }

    if((count_tokens(commands[0][0], '=') -1) > 0) {
        printf("not setting a var: %s\n", commands[0][0]);
        environ = putenv(commands[0][0], environ);
        return 1;
    }

    //check for exit
    if(strcmp(commands[0][0], "exit") == 0) {
        printf("Exiting ...\n");
        exit(0);
    }

    //check for cd
    if(strcmp(commands[0][0], "cd") == 0) {
        if(chdir(commands[0][1])) {
            fprintf(STDERR_FILENO, "no such folder: %s\n", commands[0][1]);
        }
        return 1;
    }


    return 0;
}
