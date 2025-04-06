#ifndef _STDBOOL_H
#define _STDBOOL_H

/* POSIX/C89-kompatibel bool-definition */

#ifndef __cplusplus

/* C89 har inget bool – vi fixar det själv */
#define bool    _Bool
#define true    1
#define false   0

/* Typen _Bool fanns inte i C89, så vi simulerar den */
typedef unsigned char _Bool;

#endif /* !__cplusplus */

#endif /* _STDBOOL_H */
