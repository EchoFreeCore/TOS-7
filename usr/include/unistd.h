#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>
#include <stddef.h>   // för size_t, NULL
#include <stdint.h>   // för intptr_t (sbrk)

/* File descriptors */
#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

/* POSIX.1-1988 core system call prototypes */

// File operations (syscall)
int     open(const char *pathname, int flags, ...);
int     close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t   lseek(int fd, off_t offset, int whence);

// Process control
pid_t   fork(void);
void    _exit(int status);
int     execve(const char *pathname, char *const argv[], char *const envp[]);

// High-level exec (libc wrapper)
int     execl(const char *path, const char *arg, ...);
int     execv(const char *path, char *const argv[]);
int     execle(const char *path, const char *arg, ...);
int     execvp(const char *file, char *const argv[]);
int     execlp(const char *file, const char *arg, ...);

// Pipe & descriptor duplication
int     pipe(int pipefd[2]);
int     dup(int oldfd);
int     dup2(int oldfd, int newfd);

// Working directory
int     chdir(const char *path);
int     fchdir(int fd);
char   *getcwd(char *buf, size_t size);

// Unlink / files
int     unlink(const char *pathname);

// User/group ID
uid_t   getuid(void);
uid_t   geteuid(void);
gid_t   getgid(void);
gid_t   getegid(void);

// Process identity
pid_t   getpid(void);
pid_t   getppid(void);

// File descriptor info
int     isatty(int fd);

// Memory
void   *sbrk(intptr_t increment);
int     brk(void *end_data_segment);

// Sleep
unsigned int sleep(unsigned int seconds);

// Environment (libc-level)
char   *getenv(const char *name);
int     putenv(char *string);
int     setenv(const char *name, const char *value, int overwrite);
int     unsetenv(const char *name);

#endif /* _UNISTD_H */
