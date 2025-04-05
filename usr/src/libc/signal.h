#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef int sig_atomic_t;

#define SIGINT  2
#define SIGTERM 15
#define SIGKILL 9
#define SIGSEGV 11
#define SIGCHLD 17

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);
int raise(int sig);

#endif /* _SIGNAL_H */