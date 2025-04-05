#ifndef FD_H
#define FD_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_FDS 64

/*
 * fd_type_t
 *
 * Typen av objekt som en filbeskrivare kan representera.
 * Här definierar vi FD_SOCKET för sockets, men du kan utvidga med FD_FILE etc.
 */
typedef enum {
    FD_NONE = 0,
    FD_SOCKET,
    // FD_FILE,   // Kan läggas till vid behov
} fd_type_t;

/*
 * fd_entry_t
 *
 * En filbeskrivningspost. Varje post kopplar en filbeskrivare till ett internt objekt.
 */
typedef struct {
    fd_type_t type;  // Typen på objektet (ex. FD_SOCKET)
    void *object;    // Pekare till det underliggande objektet (ex. socket_t *)
    bool used;
} fd_entry_t;

/*
 * fd_init()
 *
 * Initierar filbeskrivningstabellen. Bör anropas under systemets uppstart.
 */
void fd_init(void);

/*
 * fd_alloc()
 *
 * Allokerar en ny filbeskrivare för ett objekt med given typ.
 * Returnerar ett positivt fd-nummer vid framgång, annars -1.
 */
int fd_alloc(fd_type_t type, void *object);

/*
 * fd_get()
 *
 * Hämtar det underliggande objektet kopplat till en given filbeskrivare,
 * om typen matchar.
 */
void *fd_get(int fd, fd_type_t type);

/*
 * fd_close()
 *
 * Frigör en filbeskrivare.
 */
void fd_close(int fd);

#endif /* FD_H */
