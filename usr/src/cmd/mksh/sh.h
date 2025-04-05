#ifndef SH_H
#define SH_H

/* Standard headers från POSIX.1-1988 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

/* Typer */
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long longoff_t; /* Kan anpassas efter din libc */

/* Konstanter */
#define MKSH_VERSION "mksh-minimal"

/* Interna felkoder och kontrollflaggor */
#define EXEXIT     0
#define EXERROR    1
#define EXSIG      2
#define EXINT      3

/* Enkel hantering av job control / signal hantering */
typedef void (*sighandler_t)(int);

/* Enkla makron för allokering (kan bytas till egna allocators) */
#define alloc(n)    malloc(n)
#define aresize(p,n) realloc(p,n)
#define afree(p)    free(p)

/* IO-helpers */
#define ksh_puts(s, f)   fputs((s), (f))
#define ksh_putchar(c, f) fputc((c), (f))

/* Externa funktioner som implementeras i övriga C-filer */
void errorf(const char *, ...);
void warningf(const char *, ...);
void shell_main(int argc, char *argv[]);

#endif
