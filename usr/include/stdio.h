#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>  // för size_t
#include <stdarg.h>  // för va_list

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Symboler */
#define BUFSIZ 1024
#define EOF (-1)

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Typ */
typedef struct _FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

/* Formaterad output/input */
int printf(const char* format, ...);
int fprintf(FILE* stream, const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);

int scanf(const char* format, ...);
int fscanf(FILE* stream, const char* format, ...);
int sscanf(const char* str, const char* format, ...);

int vprintf(const char* format, va_list);
int vfprintf(FILE* stream, const char* format, va_list);
int vsprintf(char* str, const char* format, va_list);
int vsnprintf(char* str, size_t size, const char* format, va_list);

/* I/O */
int getchar(void);
int putchar(int c);
char* gets(char* s);  // OBS: osäker
int puts(const char* s);

int fgetc(FILE* stream);
int fputc(int c, FILE* stream);
char* fgets(char* s, int size, FILE* stream);
int fputs(const char* s, FILE* stream);

int getc(FILE* stream);
int putc(int c, FILE* stream);

/* Filhantering */
FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

int fseek(FILE* stream, long offset, int whence);
long ftell(FILE* stream);
void rewind(FILE* stream);

void clearerr(FILE* stream);
int feof(FILE* stream);
int ferror(FILE* stream);

int ungetc(int c, FILE* stream);
void perror(const char* s);

int setvbuf(FILE* stream, char* buf, int mode, size_t size);
void setbuf(FILE* stream, char* buf);

#endif /* _STDIO_H */

