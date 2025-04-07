#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "pipeline.h"
#include "parser.h"
#include "jobs.h"

#define MAXPIPES 8
#define MAXARGS 64

void run_piped_commands(char *line) {
    char *cmds[MAXPIPES];
    int ncmds = 0;

    char *cmd = strtok(line, "|");
    while (cmd && ncmds < MAXPIPES) {
        cmds[ncmds++] = cmd;
        cmd = strtok(NULL, "|");
    }

    int pipes[MAXPIPES - 1][2];
    for (int i = 0; i < ncmds - 1; ++i) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < ncmds; ++i) {
        char *argv[MAXARGS];
        char *infile = NULL, *outfile = NULL;
        int append = 0, background = 0;

        parse_command(cmds[i], argv, &infile, &outfile, &append, &background);
        if (!argv[0]) continue;

        // Built-in hantering här om du vill
        if (strcmp(argv[0], "cd") == 0) {
            if (argv[1]) chdir(argv[1]);
            else fprintf(stderr, "cd: missing argument\n");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            // Input från tidigare pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            // Output till nästa pipe
            if (i < ncmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Redirection
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) { perror("open infile"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (outfile) {
                int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
                int fd = open(outfile, flags, 0644);
                if (fd < 0) { perror("open outfile"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Stäng alla pipe-ändar
            for (int j = 0; j < ncmds - 1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(argv[0], argv);
            perror("exec");
            exit(1);
        }

        if (!background) add_job(pid);
    }

    // Stäng alla pipes i förälder
    for (int i = 0; i < ncmds - 1; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Vänta på alla om foreground
    for (int i = 0; i < ncmds; ++i) {
        int status;
        wait(&status);
    }
}
