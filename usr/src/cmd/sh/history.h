// I t.ex. history.h
#define MAX_HISTORY 100

char *history[MAX_HISTORY];
int history_count;

void add_to_history(const char *line);
void show_history();
const char *get_history_entry(int index);
