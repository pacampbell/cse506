#include "cmdparse.h"

static char** parse_cmd(char *cmd, char **fked) {
    char **tokens = NULL;
    if(cmd != NULL) {
        int token_count = count_tokens(cmd, ' ');
        if(token_count > 0) {
            tokens = split(cmd, ' ');
        }
    }
    return tokens;
}

char*** extract_commands(const char *cmd_str) {
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
            char **result = parse_cmd(split_commands[cmmd_i], split_commands);
            commands[cmmd_i] = result;
            // Increment the command count
            cmmd_i++;
        }
        // Add a NULL token to the end of the list
        commands[cmmd_i] = NULL;
    }
    return commands;
}
