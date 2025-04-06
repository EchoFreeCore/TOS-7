#ifndef _STDDEF_H
#define _STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* --- NULL --- */
#ifndef NULL
#define NULL ((void *)0)
#endif

/* --- size_t & ptrdiff_t (arkitekturmedveten) --- */

#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
    typedef unsigned long size_t;
    typedef long          ptrdiff_t;
#elif defined(__i386__) || defined(__arm__) || defined(_M_IX86)
    typedef unsigned int  size_t;
    typedef int           ptrdiff_t;
#else
#   error "Unsupported architecture in <stddef.h>"
#endif

/* --- offsetof --- */
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#ifdef __cplusplus
}
#endif

#endif /* _STDDEF_H */
