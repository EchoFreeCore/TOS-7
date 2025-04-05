#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>

/* POSIX.1-1988 system call prototypes */

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

extern ssize_t read(int fd, void* buf, size_t count);
extern ssize_t write(int fd, const void* buf, size_t count);
extern int close(int fd);
extern int dup(int oldfd);
extern int dup2(int oldfd, int newfd);
extern int execve(const char* pathname, char* const argv[], char* const envp[]);
extern pid_t fork(void);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern int pipe(int pipefd[2]);
extern int chdir(const char* path);
extern int fchdir(int fd);
extern int unlink(const char* pathname);
extern void _exit(int status);
extern unsigned int sleep(unsigned int seconds);
extern char* getcwd(char* buf, size_t size);
extern uid_t getuid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern gid_t getegid(void);
extern int isatty(int fd);

#endif /* _UNISTD_H */