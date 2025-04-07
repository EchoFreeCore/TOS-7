#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "alias.h"

int parse_command(char *cmd, char **argv, char **infile, char **outfile, int *append, int *background) {
    int argc = 0;
    *infile = NULL;
    *outfile = NULL;
    *append = 0;
    *background = 0;

    char *tok = strtok(cmd, " ");
    while (tok && argc < MAXARGS - 1) {
        if (strcmp(tok, "<") == 0) {
            if ((tok = strtok(NULL, " "))) *infile = tok;
        } else if (strcmp(tok, ">") == 0) {
            if ((tok = strtok(NULL, " "))) *outfile = tok;
        } else if (strcmp(tok, ">>") == 0) {
            if ((tok = strtok(NULL, " "))) {
                *outfile = tok;
                *append = 1;
            }
        } else if (strcmp(tok, "&") == 0) {
            *background = 1;
        } else {
            argv[argc++] = tok;
        }
        tok = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    if (argc > 0) {
        const char *resolved = resolve_alias(argv[0]);
        if (resolved != argv[0]) {
            argv[0] = strdup(resolved);
        }
    }

    return argc;
}