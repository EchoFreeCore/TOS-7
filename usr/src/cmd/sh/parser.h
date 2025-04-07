#ifndef PARSER_H
#define PARSER_H

#define MAXARGS 64

int parse_command(char *cmd, char **argv, char **infile, char **outfile, int *append, int *background);

#endif