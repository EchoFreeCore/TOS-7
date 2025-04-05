#include "fd.h"
#include <string.h>

static fd_entry_t fd_table[MAX_FDS];

/*
 * fd_init()
 *
 * Nollställer filbeskrivningstabellen.
 */
void fd_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
}

/*
 * fd_alloc()
 *
 * Letar efter en ledig plats i fd_table och allokerar en filbeskrivare.
 */
int fd_alloc(fd_type_t type, void *object) {
    for (int i = 0; i < MAX_FDS; i++) {
        if (!fd_table[i].used) {
            fd_table[i].used = true;
            fd_table[i].type = type;
            fd_table[i].object = object;
            return i;
        }
    }
    return -1;  // Inga lediga fds
}

/*
 * fd_get()
 *
 * Returnerar det underliggande objektet om fd är giltig och typen matchar.
 */
void *fd_get(int fd, fd_type_t type) {
    if (fd < 0 || fd >= MAX_FDS)
        return 0;
    if (!fd_table[fd].used || fd_table[fd].type != type)
        return 0;
    return fd_table[fd].object;
}

/*
 * fd_close()
 *
 * Frigör filbeskrivningen och nollställer posten.
 */
void fd_close(int fd) {
    if (fd < 0 || fd >= MAX_FDS)
        return;
    fd_table[fd].used = false;
    fd_table[fd].type = FD_NONE;
    fd_table[fd].object = 0;
}
