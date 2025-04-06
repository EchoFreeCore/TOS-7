#ifndef _STDINT_H
#define _STDINT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 1. Arkitekturdetektering ---- */
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
#  define __ARCH_64BIT 1
#elif defined(__i386__) || defined(_M_IX86) || defined(__arm__)
#  define __ARCH_32BIT 1
#else
#  error "Unsupported architecture – extend stdint.h"
#endif

/* ---- 2. Fast-breddstyper ---- */

typedef signed char        int8_t;
typedef unsigned char      uint8_t;

typedef short              int16_t;
typedef unsigned short     uint16_t;

typedef int                int32_t;
typedef unsigned int       uint32_t;

#if defined(__GNUC__) || defined(__clang__)
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#else
#  error "64-bit types require compiler support for 'long long'"
#endif

/* ---- 3. Min/max värden (optional för dig, POSIX-style) ---- */
#define INT8_MIN   (-128)
#define INT8_MAX   (127)
#define UINT8_MAX  (255U)

#define INT16_MIN  (-32768)
#define INT16_MAX  (32767)
#define UINT16_MAX (65535U)

#define INT32_MIN  (-2147483647 - 1)
#define INT32_MAX  (2147483647)
#define UINT32_MAX (4294967295U)

#define INT64_MIN  (-9223372036854775807LL - 1)
#define INT64_MAX  (9223372036854775807LL)
#define UINT64_MAX (18446744073709551615ULL)

/* ---- 4. Maskinbreddsberoende typer ---- */

#if defined(__ARCH_64BIT)
typedef long               intptr_t;
typedef unsigned long      uintptr_t;
typedef long               intmax_t;
typedef unsigned long      uintmax_t;
#elif defined(__ARCH_32BIT)
typedef int                intptr_t;
typedef unsigned int       uintptr_t;
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
#endif

/* ---- 5. Största möjliga bredder ---- */
#define INTPTR_MIN  ((intptr_t)(~((uintptr_t)0) >> 1) + 1)
#define INTPTR_MAX  ((intptr_t)(~((uintptr_t)0) >> 1))
#define UINTPTR_MAX ((uintptr_t)~((uintptr_t)0))

#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

#ifdef __cplusplus
}
#endif

#endif /* _STDINT_H */
