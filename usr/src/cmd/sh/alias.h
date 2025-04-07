#ifndef ALIAS_H
#define ALIAS_H

void set_alias(const char *name, const char *value);
void remove_alias(const char *name);
const char* resolve_alias(const char *name);
void handle_alias_command(char **argv);
void print_aliases();
void cleanup_aliases();

#endif