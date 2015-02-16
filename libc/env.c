#include <sys/defs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

    //memcpy(new_env, envp, i);
    for(int j = 0; j < i; j++) {
        new_env[j] = envp[j];
    }

    if(var == NULL) return envp;

    new_env[i] = malloc(varlen * sizeof(char));
    if(new_env[1] == NULL) {
        fprintf(STDERR_FILENO, "malloc failed in putenv()\n");
        return envp;
    }

    new_env[i+1] = NULL;
    memcpy(new_env[i], var, varlen);
    new_env[i][varlen] = '\0';

    free(envp);
    return new_env;
}

char **rmenv(char *var, char **envp) {
    int i;
    int index;
    for(i = 0; *(envp + i) != NULL; i++);

    index = find_env_var_orig(envp, var);
    if(envp[index] == NULL) return envp;

    //last env
    if(envp[index] == *(envp + i - 1)) {
        envp[i-1] = NULL;
        return envp;
    }

    envp[index] = envp[i-1];
    envp[i-1] = NULL;

    return envp;
}

char **setenv(char *var, char **envp) {
    int i;
    char *c;
    char *eq;

    for(i = 0; *(envp + i) != NULL; i++);
    for(eq= var; *eq != '\0' && *eq != '='; eq++);
    *eq = '\0';
    
    c = find_env_var(envp, var);
    *eq = '=';

    if(c == NULL) {
        return putenv(var, envp);
    } else {
        *eq = '\0';
        envp = rmenv(var, envp);
        *eq = '=';
        return putenv(var, envp);        
    }
}
