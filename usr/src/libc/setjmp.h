#ifndef _SETJMP_H
#define _SETJMP_H

/* Non-local goto interface */

typedef int jmp_buf[16];

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

#endif /* _SETJMP_H */