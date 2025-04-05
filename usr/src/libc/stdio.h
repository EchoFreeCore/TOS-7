#ifndef _STDIO_H
#define _STDIO_H

/* Minimal FILE and I/O interface */

typedef struct _FILE FILE;
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int printf(const char* format, ...);
int fprintf(FILE* stream, const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);

int putchar(int c);
int getchar(void);
int puts(const char* s);
char* gets(char* s);

#endif /* _STDIO_H */