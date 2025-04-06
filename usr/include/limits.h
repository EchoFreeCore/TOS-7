#ifndef _LIMITS_H
#define _LIMITS_H

/* --- Standard C limits --- */

#define CHAR_BIT    8

#define SCHAR_MIN   (-128)
#define SCHAR_MAX   127
#define UCHAR_MAX   255

#define CHAR_MIN    SCHAR_MIN
#define CHAR_MAX    SCHAR_MAX

#define SHRT_MIN    (-32768)
#define SHRT_MAX    32767
#define USHRT_MAX   65535

#define INT_MIN     (-2147483647 - 1)
#define INT_MAX     2147483647
#define UINT_MAX    4294967295U

/* --- Long/Long Long beroende på arkitektur --- */

#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
#  define LONG_MAX     9223372036854775807L
#  define LONG_MIN     (-LONG_MAX - 1L)
#  define ULONG_MAX    18446744073709551615UL
#else
#  define LONG_MAX     2147483647L
#  define LONG_MIN     (-LONG_MAX - 1L)
#  define ULONG_MAX    4294967295UL
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define LLONG_MAX    9223372036854775807LL
#  define LLONG_MIN    (-LLONG_MAX - 1LL)
#  define ULLONG_MAX   18446744073709551615ULL
#endif

/* --- POSIX.1-1988 constants --- */

#define NAME_MAX    255     // max length of filename
#define PATH_MAX    1024    // max length of full path
#define LINK_MAX    127     // max # of links to a file
#define PIPE_BUF    512     // atomic write to pipe
#define _POSIX_ARG_MAX 4096 // min bytes for args + env to execve
#define _POSIX_CHILD_MAX 6  // min simultaneous processes per user

#endif /* _LIMITS_H */
