#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include "syscall.h"
#include "syscall_nb.h"

int open(const char *pathname, int flags, ...) {
    int mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return (int)syscall(SYS_open, (uintptr_t)pathname, (uintptr_t)flags, (uintptr_t)mode, 0, 0, 0);
}

int close(int fd) {
    return (int)syscall(SYS_close, (uintptr_t)fd, 0, 0, 0, 0, 0);
}

ssize_t read(int fd, void *buf, size_t count) {
    return (ssize_t)syscall(SYS_read, (uintptr_t)fd, (uintptr_t)buf, (uintptr_t)count, 0, 0, 0);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return (ssize_t)syscall(SYS_write, (uintptr_t)fd, (uintptr_t)buf, (uintptr_t)count, 0, 0, 0);
}

off_t lseek(int fd, off_t offset, int whence) {
    return (off_t)syscall(SYS_lseek, (uintptr_t)fd, (uintptr_t)offset, (uintptr_t)whence, 0, 0, 0);
}

pid_t fork(void) {
    return (pid_t)syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
}

void _exit(int status) {
    syscall(SYS_exit, (uintptr_t)status, 0, 0, 0, 0, 0);
    __builtin_unreachable();
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    return (int)syscall(SYS_execve, (uintptr_t)pathname, (uintptr_t)argv, (uintptr_t)envp, 0, 0, 0);
}

int execl(const char *path, const char *arg, ...) {
    va_list ap;
    va_start(ap, arg);
    const char *argv[64];
    int i = 0;
    argv[i++] = arg;
    const char *p;
    while ((p = va_arg(ap, const char *)) != NULL && i < 63)
        argv[i++] = p;
    argv[i] = NULL;
    va_end(ap);
    return execv(path, (char *const *)argv);
}

int execv(const char *path, char *const argv[]) {
    return execve(path, argv, (char *const *)NULL);
}

int execle(const char *path, const char *arg, ...) {
    va_list ap;
    va_start(ap, arg);
    const char *argv[64];
    int i = 0;
    argv[i++] = arg;
    const char *p;
    while ((p = va_arg(ap, const char *)) != NULL && i < 63)
        argv[i++] = p;
    argv[i] = NULL;
    char *const *envp = va_arg(ap, char *const *);
    va_end(ap);
    return execve(path, (char *const *)argv, envp);
}

int execvp(const char *file, char *const argv[]) {
    // En enkel fallback för att testa olika PATH-värden kan implementeras här.
    return execve(file, argv, (char *const *)NULL);
}

int execlp(const char *file, const char *arg, ...) {
    va_list ap;
    va_start(ap, arg);
    const char *argv[64];
    int i = 0;
    argv[i++] = arg;
    const char *p;
    while ((p = va_arg(ap, const char *)) != NULL && i < 63)
        argv[i++] = p;
    argv[i] = NULL;
    va_end(ap);
    return execvp(file, (char *const *)argv);
}

int pipe(int pipefd[2]) {
    return (int)syscall(SYS_pipe, (uintptr_t)pipefd, 0, 0, 0, 0, 0);
}

int dup(int oldfd) {
    return (int)syscall(SYS_dup, (uintptr_t)oldfd, 0, 0, 0, 0, 0);
}

int dup2(int oldfd, int newfd) {
    return (int)syscall(SYS_dup2, (uintptr_t)oldfd, (uintptr_t)newfd, 0, 0, 0, 0);
}

int chdir(const char *path) {
    return (int)syscall(SYS_chdir, (uintptr_t)path, 0, 0, 0, 0, 0);
}

int fchdir(int fd) {
    return (int)syscall(SYS_fchdir, (uintptr_t)fd, 0, 0, 0, 0, 0);
}

char *getcwd(char *buf, size_t size) {
    long ret = syscall(SYS_getcwd, (uintptr_t)buf, (uintptr_t)size, 0, 0, 0, 0);
    return ret < 0 ? NULL : buf;
}

int unlink(const char *pathname) {
    return (int)syscall(SYS_unlink, (uintptr_t)pathname, 0, 0, 0, 0, 0);
}

uid_t getuid(void) {
    return (uid_t)syscall(SYS_getuid, 0, 0, 0, 0, 0, 0);
}

uid_t geteuid(void) {
    return (uid_t)syscall(SYS_geteuid, 0, 0, 0, 0, 0, 0);
}

gid_t getgid(void) {
    return (gid_t)syscall(SYS_getgid, 0, 0, 0, 0, 0, 0);
}

gid_t getegid(void) {
    return (gid_t)syscall(SYS_getegid, 0, 0, 0, 0, 0, 0);
}

pid_t getpid(void) {
    return (pid_t)syscall(SYS_getpid, 0, 0, 0, 0, 0, 0);
}

pid_t getppid(void) {
    return (pid_t)syscall(SYS_getppid, 0, 0, 0, 0, 0, 0);
}

int isatty(int fd) {
    // Du kan göra en ioctl här, men vi kör bara en förenklad variant
    return (int)syscall(SYS_isatty, (uintptr_t)fd, 0, 0, 0, 0, 0);
}

void *sbrk(intptr_t increment) {
    static void *curbrk = 0;
    if (brk(curbrk + increment) < 0)
        return (void *)-1;
    void *old = curbrk;
    curbrk += increment;
    return old;
}

int brk(void *end_data_segment) {
    return (int)syscall(SYS_brk, (uintptr_t)end_data_segment, 0, 0, 0, 0, 0);
}

unsigned int sleep(unsigned int seconds) {
    struct timespec req = { .tv_sec = seconds, .tv_nsec = 0 };
    syscall(SYS_nanosleep, (uintptr_t)&req, 0, 0, 0, 0, 0);
    return 0;
}


