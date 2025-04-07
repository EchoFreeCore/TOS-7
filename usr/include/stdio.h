#ifndef _STDIO_H
#define _STDIO_H

typedef struct {
    int fd;
    int flags;
    int bufmode;
    char* buffer;
    char internal[BUFSIZ];
    size_t bufpos, buflen;
    int last_op;  // 1=read, 2=write
    int error;
    int eof;
    int unget;
    int bufuser;
    size_t bufsize;
} FILE;

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

FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
int fprintf(FILE *stream, const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);
int scanf(const char *format, ...);
int getchar(void);
int putchar(int c);
char *gets(char *s);
int puts(const char *s);
void perror(const char *s);

#endif /* _STDIO_H */
