#ifndef _STDLIB_H
#define _STDLIB_H

/* POSIX.1-1988 minimal standard utilities */

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void exit(int status);
void abort(void);
int atoi(const char* nptr);
long strtol(const char* nptr, char** endptr, int base);
void* malloc(size_t size);
void free(void* ptr);

#endif /* _STDLIB_H */