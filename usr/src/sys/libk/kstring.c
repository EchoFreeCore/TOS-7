#include "kstring.h"
#include "kalloc.h" // för strdup()

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while ((*dst++ = *src++));
    return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dst[i] = src[i];
    for (; i < n; i++) dst[i] = '\0';
    return dst;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(unsigned char *)a - *(unsigned char *)b;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return c == 0 ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char *)(c == 0 ? s : last);
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

// strtok() implementation: basic and not thread-safe
char *strtok(char *str, const char *delim) {
    static char *saved = 0;
    if (str) saved = str;
    if (!saved) return NULL;

    // Skip leading delimiters
    while (*saved && strchr(delim, *saved)) saved++;
    if (!*saved) return NULL;

    char *start = saved;

    // Find next delimiter
    while (*saved && !strchr(delim, *saved)) saved++;

    if (*saved) {
        *saved = '\0';
        saved++;
    } else {
        saved = NULL;
    }

    return start;
}

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len); // Använder din egen malloc
    if (!copy) return NULL;
    memcpy(copy, s, len);
    return copy;
}
