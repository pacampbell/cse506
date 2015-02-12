#include "sbush.h"

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
    // char cmd[256] = {0};
    char *cmd = "ls -l | grep crt | echo hello";
    // char *name[] = {"fake", NULL};
    char *path = NULL;
    // char *ps1;
    int rc = 0;
    char *c = NULL;

    // ps1 = find_env_var(envp, "PS1");

    /*
    for(c = *envp, rc = 0; c != NULL; rc++, c = *(envp + rc)) {
        printf("ENV::: %s\n", c);
    }
    */

    /*
    if(ps1 == NULL) {
        printf("> ");
        read(STDIN_FILENO, cmd, 256);
    } else {
        printf("%s", ps1);
    }
    */

    printf("> %s\n", cmd);

    char **parsed_command = NULL; // parse_cmd(cmd);
    char ***commands = extract_commands(cmd);
    while(*commands != NULL) {
        // Get the current command in the interation
        parsed_command = *commands;
        for(int i = 0; parsed_command[i] != NULL; i++) {
            printf("cmd> %s\n", parsed_command[i]);
        }
        // Get the next command in the list
        commands++;
    }

    //find the path var
    path = find_env_var(envp, "PATH");

    //printf("path: %s\n", path);
    rc = execve(parsed_command[0], parsed_command, envp);
    c = strtok(path, ':');
    while(c != NULL) {
        //printf("trying: %s\n", strappend(c, "/", cmd));
        rc = execve(strappend(c, "/", parsed_command[0]), parsed_command, envp);
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
