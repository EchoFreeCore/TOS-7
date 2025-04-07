#include <signal.h>
#include "readline.h"
#include "parser.h"
#include "alias.h"
#include "jobs.h"
#include "utils.h"
#include "pipeline.h"

int main() {
    signal(SIGINT, sigint_handler);
	init_jobs();
    enable_readline_support();
    run_shell();
	cleanup_aliases();
    return 0;
}

void cleanup_aliases() {
    for (int i = 0; i < alias_count; ++i) {
        free(aliases[i].name);
        free(aliases[i].value);
    }
}

void run_single_command(char *line) {
    char *argv[MAXARGS];
    char *infile = NULL, *outfile = NULL;
    int append = 0, background = 0;

    parse_command(line, argv, &infile, &outfile, &append, &background);
    if (!argv[0]) return;

    // Built-in: cd
    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1]) chdir(argv[1]);
        else fprintf(stderr, "cd: missing argument\n");
        return;
    }

    // Built-in: alias
    if (strcmp(argv[0], "alias") == 0) {
        handle_alias_command(argv);
        return;
    }

    // Built-in: unalias
    if (strcmp(argv[0], "unalias") == 0) {
        if (argv[1]) remove_alias(argv[1]);
        else fprintf(stderr, "unalias: missing argument\n");
        return;
    }

    // Built-in: jobs
    if (strcmp(argv[0], "jobs") == 0) {
        print_jobs();
        return;
    }

    // Built-in: fg
    if (strcmp(argv[0], "fg") == 0) {
        if (argv[1]) {
            int job_id = atoi(argv[1]);
            bring_job_foreground(job_id);
        } else {
            fprintf(stderr, "fg: missing job id\n");
        }
        return;
    }

    // Built-in: bg
    if (strcmp(argv[0], "bg") == 0) {
        if (argv[1]) {
            int job_id = atoi(argv[1]);
            continue_job_background(job_id);
        } else {
            fprintf(stderr, "bg: missing job id\n");
        }
        return;
    }

    // Built-in: kill
    if (strcmp(argv[0], "kill") == 0) {
        if (argv[1]) {
            int job_id = atoi(argv[1]);
            kill_job(job_id);
        } else {
            fprintf(stderr, "kill: missing job id\n");
        }
        return;
    }

    // Built-in: history
    if (strcmp(argv[0], "history") == 0) {
        show_history();
        return;
    }

    // Built-in: !N (run from history)
    if (argv[0][0] == '!' && argv[0][1] != '\0') {
        int idx = atoi(argv[0] + 1);
        const char *cmd = get_history_entry(idx);
        if (cmd) {
            printf("%s\n", cmd);
            run_single_command(strdup(cmd)); // viktigt: kopiera
        } else {
            fprintf(stderr, "No such command in history: %d\n", idx);
        }
        return;
    }

    // External command
    pid_t pid = fork();
    if (pid == 0) {
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

        execvp(argv[0], argv);
        perror("exec");
        exit(1);
    }

    if (!background) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        add_job(pid);
    }
}


void run_shell() {
    while (1) {
        char *line = read_input_line();
        if (!line) break;

        if (*line == '\0') {
            free(line);
            continue;
        }

        // Hantera exit direkt
        if (strcmp(line, "exit") == 0) {
            free(line);
            break;
        }

        // Hantera pipelining (minst ett '|')
        if (strchr(line, '|')) {
            run_piped_commands(line);
            free(line);
            continue;
        }

        if (*line != '\0') {
            add_to_history(line);
        }

        run_single_command(line);

        free(line);
    }
}
