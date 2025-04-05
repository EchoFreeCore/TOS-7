// usr/src/bin/init.c — First userspace process in TOS-X
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main(void) {
    const char *rc_argv[] = { "/bin/sh", "/etc/rc", NULL };
    const char *rc_envp[] = { "PATH=/bin:/usr/bin", NULL };

    // Print to kernel console
    write(1, "init: launching /etc/rc\n", 24);

    pid_t pid = fork();
    if (pid == 0) {
        // Child: run rc script
        execve("/bin/sh", (char *const *)rc_argv, (char *const *)rc_envp);
        _exit(1); // If exec fails
    }

    // Parent: wait for rc to finish
    int status = 0;
    waitpid(pid, &status, 0);

    // Start an interactive shell
    const char *sh_argv[] = { "/bin/sh", NULL };
    while (1) {
        write(1, "init: starting shell\n", 22);
        fork() == 0 ? execve("/bin/sh", (char *const *)sh_argv, (char *const *)rc_envp) : wait(NULL);
    }

    return 0;
}
