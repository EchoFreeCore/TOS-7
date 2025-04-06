#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>  // för size_t

#ifdef __cplusplus
extern "C" {
#endif

// === Memory operations ===
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

// === String length and comparison ===
size_t strlen(const char *s);
int    strcmp(const char *s1, const char *s2);

// === String copy ===
char  *strcpy(char *dest, const char *src);
char  *strncpy(char *dest, const char *src, size_t n);

// === Character search ===
char  *strchr(const char *s, int c);
char  *strrchr(const char *s, int c);

// === Tokenization ===
char  *strtok(char *str, const char *delim);

// === Duplicate ===
char  *strdup(const char *s);

#ifdef __cplusplus
}
#endif

#endif // KSTRING_H