#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef int sig_atomic_t;

#define SIGINT  2
#define SIGTERM 15
#define SIGKILL 9
#define SIGSEGV 11
#define SIGCHLD 17

typedef void (*sighandler_t)(int);

sighandler_t signal(int sig, sighandler_t func);
int kill(pid_t pid, int sig);
int raise(int sig);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);


#endif /* _SIGNAL_H */