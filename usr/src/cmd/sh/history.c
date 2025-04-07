#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

char *history[MAX_HISTORY];
int history_count = 0;

void add_to_history(const char *line) {
    if (history_count == MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, sizeof(char *) * (MAX_HISTORY - 1));
        history_count--;
    }
    history[history_count++] = strdup(line);
}

void show_history() {
    for (int i = 0; i < history_count; ++i) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}

const char *get_history_entry(int index) {
    if (index < 1 || index > history_count) return NULL;
    return history[index - 1];
}