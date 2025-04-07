#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

/* POSIX.1-1988 data types with 32/64-bit awareness */

/* --- Maskinberoende typer --- */
#if defined(__x86_64__) || defined(__aarch64__) || defined(__LP64__) || defined(_M_X64)
    typedef unsigned long   size_t;
    typedef long            ssize_t;
    typedef long            off_t;
    typedef long            time_t;
    typedef long            ptrdiff_t;
    typedef unsigned long   ino_t;
    typedef unsigned long   dev_t;
#else
    typedef unsigned int    size_t;
    typedef int             ssize_t;
    typedef long            off_t;       /* fortfarande long pga POSIX */
    typedef long            time_t;
    typedef int             ptrdiff_t;
    typedef unsigned long   ino_t;
    typedef unsigned int    dev_t;
#endif

/* --- POSIX-definierade typer --- */
typedef int             pid_t;
typedef unsigned int    uid_t;
typedef unsigned int    gid_t;
typedef unsigned short  mode_t;
typedef unsigned short  nlink_t;
typedef long            clock_t;

#endif /* _SYS_TYPES_H */

