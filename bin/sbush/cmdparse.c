#include "cmdparse.h"

/**
 * Call first time with string
 * Call second time with NULL
 * Ex:
 * indexof("he$ll$o", '$'); // Returns address of first $
 * indexof(NULL, '$');     // Returns address of second $
 * indexof(NULL, '$');     // Returns NULL
 */
static char* indexof(char *str, char symbol) {
    char *index = NULL;
    static char *cur;
    // If the string is not null set the static buffer
    if(str != NULL) {
        cur = str;
    }
    // Now search for the token
    if(cur != NULL) {
        while(*cur != '\0') {
            if(*cur == symbol) {
                index = cur; // Set the index to this token
                cur++;       // Move to the next character
                break;
            }
            cur++;
        }
    }
    return index;
}

static char** parse_cmd(char *cmd, char **envp) {
    char **tokens = NULL;
    if(cmd != NULL) {
        int token_count = count_tokens(cmd, ' ');
        if(token_count > 0) {
            tokens = split(cmd, ' ');
            // Check for variable expansions
            for(int i = 0; tokens[i] != NULL; i++) {
                char *found = indexof(tokens[i], '$');
                char *prev_start = tokens[i];
                // Location to hold expanded variable
                char *expanded = NULL;
                // Count total space needed to allocate
                int total_space = 0;
                while(found != NULL) {
                    // Figure out the first part of string we need
                    total_space += found - prev_start;
                    // Find end of the token
                    char *tp = found + 1;
                    // Find the end of string or start of next token
                    while(*tp != '\0' && *tp != '$') tp++;
                    // Allocate temp space for variable
                    char variable[tp - (found+1)];
                    strncpy(variable, (found+1), tp - (found+1));
                    // Null terminate the temp var
                    variable[tp - (found+1)] = '\0';
                    // Search for the variable
                    char *env = find_env_var(envp, variable);
                    if(env != NULL) {
                        if(expanded == NULL) {
                            expanded = malloc(total_space + strlen(env));
                            strncpy(expanded, tokens[i], total_space);
                            // Copy the env in after
                            strncpy(expanded + total_space, env, strlen(env));
                        } else {
                            // Copy the old string
                            int old_len = strlen(expanded) - 1;
                            char old[old_len];
                            strcpy(old, expanded);
                            // Release the old string
                            free(expanded);
                            expanded = malloc(total_space + strlen(env));
                            // Copy in first part
                            strncpy(expanded, old, old_len);
                            // Now copy in the new part
                            strncpy(expanded + old_len, env, strlen(env));
                        }
                    }
                    // Now figure out if we found a new variable or end of string
                    found = indexof(NULL, '$');
                }
                if(expanded != NULL) {
                    // Set the new string if it changed
                    if(strcmp(tokens[i], expanded)) {
                        tokens[i] = expanded;
                    }
                }
            }
        }
    }
    return tokens;
}

char*** extract_commands(const char *cmd_str, char *envp[]) {
    char ***commands = NULL;
    if(cmd_str != NULL) {
        // current command index
        int cmmd_i = 0;
        // Count the number of expected commands
        int command_count = count_tokens(cmd_str, '|');
        // Allocate space for the list of commands + 1 for NULL
        commands = malloc(sizeof(char*) * (command_count + 1));
        // Split the token on the pipes
        char **split_commands = split(cmd_str, '|');
        for(int i = 0; split_commands[i] != NULL; i++) {
            split_commands[i] = strip(split_commands[i]);
        }
        // Parse each line
        while(split_commands[cmmd_i] != NULL) {
            // Parse the individual command
            char **result = parse_cmd(split_commands[cmmd_i], envp);
            commands[cmmd_i] = result;
            // Increment the command count
            cmmd_i++;
        }
        // Add a NULL token to the end of the list
        commands[cmmd_i] = NULL;
    }
    return commands;
}
