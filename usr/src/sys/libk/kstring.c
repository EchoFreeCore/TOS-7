#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>

size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strdup(const char *s);
char *strtok(char *str, const char *delim);
int strcmp(const char *a, const char *b);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);

#endif
