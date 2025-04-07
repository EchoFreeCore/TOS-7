#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alias.h"

#define MAXALIASES 64

struct alias {
    char *name;
    char *value;
} aliases[MAXALIASES];

int alias_count = 0;

const char* resolve_alias(const char *name) {
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].value;
        }
    }
    return name;
}

void set_alias(const char *name, const char *value) {
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].value);
            aliases[i].value = strdup(value);
            return;
        }
    }
    if (alias_count < MAXALIASES) {
        aliases[alias_count].name = strdup(name);
        aliases[alias_count].value = strdup(value);
        alias_count++;
    }
}

void remove_alias(const char *name) {
    for (int i = 0; i < alias_count; ++i) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].name);
            free(aliases[i].value);
            for (int j = i; j < alias_count - 1; ++j) {
                aliases[j] = aliases[j + 1];
            }
            alias_count--;
            return;
        }
    }
    fprintf(stderr, "unalias: %s: not found\n", name);
}

void print_aliases() {
    for (int i = 0; i < alias_count; ++i) {
        printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
    }
}

void handle_alias_command(char **argv) {
    if (!argv[1]) {
        print_aliases();
    } else {
        for (int i = 1; argv[i]; ++i) {
            char *arg = argv[i];
            char *eq = strchr(arg, '=');
            if (eq) {
                *eq = '\0';
                char *name = arg;
                char *value = eq + 1;
                if (*value == '\'' || *value == '"') {
                    value++;
                    char *end = strchr(value, '\0');
                    if (end && *(end - 1) == *(value - 1)) *(end - 1) = '\0';
                }
                set_alias(name, value);
            } else {
                const char *resolved = resolve_alias(arg);
                if (resolved != arg) {
                    printf("alias %s='%s'\n", arg, resolved);
                } else {
                    fprintf(stderr, "alias: %s: not found\n", arg);
                }
            }
        }
    }
}
