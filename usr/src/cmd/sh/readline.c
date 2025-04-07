#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

void enable_readline_support() {
    rl_bind_key('\t', rl_complete);
    using_history();
    rl_attempted_completion_function = my_completion;
}

char *read_input_line() {
    char *line = readline("$ ");
    if (line && *line) add_history(line);
    return line;
}

// ---- Completion logic ----

char* command_generator(const char* text, int state) {
    static DIR* dir;
    static struct dirent* ent;
    static int pass;
    const char* dirs[] = {"/bin", "/usr/bin", NULL};

    if (state == 0) {
        pass = 0;
        if (dir) closedir(dir);
        dir = opendir(dirs[pass]);
    }

    while (dir) {
        while ((ent = readdir(dir))) {
            if (strncmp(ent->d_name, text, strlen(text)) == 0) {
                return strdup(ent->d_name);
            }
        }
        closedir(dir);
        dir = NULL;
        if (dirs[++pass]) {
            dir = opendir(dirs[pass]);
        }
    }

    return NULL;
}

char** my_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 0;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, filename_completion_function);
}
