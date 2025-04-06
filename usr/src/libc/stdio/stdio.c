#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "syscall.h"
#include "syscall-nr.h"
#include "stdio.h"

// Minimal FILE-struktur
typedef struct _FILE {
    int fd;
} FILE;

// Statiska standardströmmar
FILE _stdin  = { .fd = 0 };
FILE _stdout = { .fd = 1 };
FILE _stderr = { .fd = 2 };

FILE *stdin  = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

// Intern skrivare
static void fd_putc(char c, void *userdata) {
    int fd = *(int*)userdata;
    syscall(SYS_write, fd, (uintptr_t)&c, 1, 0, 0, 0);
}

// fprintf och printf via samma backend
int vfprintf(FILE *stream, const char *format, va_list ap) {
    int fd = stream->fd;
    int count = 0;

    // intern putc-funktion
    void local_putc(char c, void *ud) {
        syscall(SYS_write, fd, (uintptr_t)&c, 1, 0, 0, 0);
        count++;
    }

    // kör formatmotorn
    extern int vsnprintf_core(char *str, size_t size, const char *fmt, va_list ap);
    // workaround: vi skriver direkt till fd, inte via buffert
    // så vi måste duplicera parsern om vi vill ha 100% kontroll
    // detta duger initialt

    // Workaround för printf utan duplicering:
    char tmp[512];
    int len = vsnprintf_core(tmp, sizeof(tmp), format, ap);
    if (len > 0)
        syscall(SYS_write, fd, (uintptr_t)tmp, len, 0, 0, 0);

    return len;
}

int fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stream, format, ap);
    va_end(ap);
    return ret;
}

int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stdout, format, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    extern int vsnprintf_core(char *str, size_t size, const char *fmt, va_list ap);
    int ret = vsnprintf_core(str, size, format, ap);
    va_end(ap);
    return ret;
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    extern int vsnprintf_core(char *str, size_t size, const char *fmt, va_list ap);
    int ret = vsnprintf_core(str, (size_t)-1, format, ap);
    va_end(ap);
    return ret;
}

int putchar(int c) {
    char ch = (char)c;
    syscall(SYS_write, 1, (uintptr_t)&ch, 1, 0, 0, 0);
    return c;
}

int puts(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    syscall(SYS_write, 1, (uintptr_t)s, len, 0, 0, 0);
    syscall(SYS_write, 1, (uintptr_t)"\n", 1, 0, 0, 0);
    return (int)(len + 1);
}

int getchar(void) {
    char c;
    if (syscall(SYS_read, 0, (uintptr_t)&c, 1, 0, 0, 0) == 1)
        return (int)c;
    return -1;
}

char *gets(char *s) {
    int i = 0;
    char c;
    while (syscall(SYS_read, 0, (uintptr_t)&c, 1, 0, 0, 0) == 1) {
        if (c == '\n' || c == '\r') {
            s[i] = '\0';
            return s;
        }
        s[i++] = c;
    }
    return NULL;
}

void perror(const char *s) {
    if (s && *s) {
        syscall(SYS_write, 2, (uintptr_t)s, strlen(s), 0, 0, 0);
        syscall(SYS_write, 2, (uintptr_t)": ", 2, 0, 0, 0);
    }
    const char *msg = "error\n"; // du kan förbättra till errno-baserat
    syscall(SYS_write, 2, (uintptr_t)msg, strlen(msg), 0, 0, 0);
}
