#include <sys/defs.h>
#include <string.h>
#include <stdio.h>

char **putenv(char *var, char **envp) {

    int varlen = strlen(var);
    char **new_env;
    //count how many vars are in envp
    int i;
    for(i = 0; *(envp + i) != NULL; i++);

    if(var == NULL) i--;

    new_env = malloc((i + 1) * sizeof(char *));
    if(new_env == NULL) {
        fprintf(STDERR_FILENO, "malloc failed in putenv()\n");
        return envp;
    }

    memcpy(new_env, envp, i);
    if(var == NULL) return envp;

    new_env[i] = malloc(varlen * sizeof(char));
    if(new_env[1] == NULL) {
        fprintf(STDERR_FILENO, "malloc failed in putenv()\n");
        return envp;
    }

    new_env[i+1] = NULL;
    memcpy(new_env[i], var, varlen);
    new_env[i][varlen] = '\0';

    return new_env;
}
