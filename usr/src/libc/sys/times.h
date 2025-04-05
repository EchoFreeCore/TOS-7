#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#include <sys/types.h>

struct tms {
    clock_t tms_utime;   /* user CPU time */
    clock_t tms_stime;   /* system CPU time */
    clock_t tms_cutime;  /* user CPU time of children */
    clock_t tms_cstime;  /* system CPU time of children */
};

clock_t times(struct tms* buf);

#endif /* _SYS_TIMES_H */


#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>

pid_t wait(int* status);
pid_t waitpid(pid_t pid, int* status, int options);

#define WNOHANG    1
#define WUNTRACED  2

#define WIFEXITED(status)   (((status) & 0x7F) == 0)
#define WEXITSTATUS(status) (((status) >> 8) & 0xFF)
#define WIFSIGNALED(status) (((status) & 0x7F) != 0 && ((status) & 0x7F) != 0x7F)
#define WTERMSIG(status)    ((status) & 0x7F)

#endif /* _SYS_WAIT_H */



