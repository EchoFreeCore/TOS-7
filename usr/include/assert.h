#ifndef _ASSERT_H
#define _ASSERT_H

/* Minimal assert macro for C89 */
#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) ((expr) ? (void)0 : __assert_fail(#expr, __FILE__, __LINE__))
extern void __assert_fail(const char* expr, const char* file, int line);
#endif

#endif /* _ASSERT_H */