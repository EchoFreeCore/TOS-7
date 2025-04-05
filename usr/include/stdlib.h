#ifndef _STDLIB_H
#define _STDLIB_H

/* POSIX.1-1988 minimal standard utilities */

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void exit(int status);
void abort(void);
int atoi(const char* nptr);
long strtol(const char* nptr, char** endptr, int base);

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void exit(int status);
int atexit(void (*function)(void));
int atoi(const char *nptr);
long atol(const char *nptr);
long strtol(const char *nptr, char **endptr, int base);

#endif /* _STDLIB_H */
