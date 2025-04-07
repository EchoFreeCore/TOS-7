#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

void init_jobs();
void add_job(pid_t pid);
void remove_job(pid_t pid);
void print_jobs();
void bring_job_foreground(int job_id);
void continue_job_background(int job_id);
void kill_job(int job_id);

#endif

// jobs.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "jobs.h"

#define MAXJOBS 128

pid_t jobs[MAXJOBS];
int job_count = 0;

void init_jobs() {
    job_count = 0;
    memset(jobs, 0, sizeof(jobs));
}

void add_job(pid_t pid) {
    if (job_count < MAXJOBS) {
        jobs[job_count++] = pid;
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; ++i) {
        if (jobs[i] == pid) {
            for (int j = i; j < job_count - 1; ++j)
                jobs[j] = jobs[j + 1];
            job_count--;
            break;
        }
    }
}

void print_jobs() {
    for (int i = 0; i < job_count; ++i) {
        int status;
        pid_t result = waitpid(jobs[i], &status, WNOHANG | WUNTRACED | WCONTINUED);
        const char *state = "Unknown";

        if (result == 0) {
            state = "Running";
        } else if (WIFSTOPPED(status)) {
            state = "Stopped";
        } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
            state = "Done";
            remove_job(jobs[i]);
            continue;
        }

        printf("[%d] %d\t%s\n", i + 1, jobs[i], state);
    }
}

void bring_job_foreground(int job_id) {
    if (job_id < 1 || job_id > job_count) {
        fprintf(stderr, "fg: no such job %d\n", job_id);
        return;
    }

    pid_t pid = jobs[job_id - 1];

    // Fortsätt om stoppad
    kill(pid, SIGCONT);

    int status;
    waitpid(pid, &status, 0);

    remove_job(pid);
}

void continue_job_background(int job_id) {
    if (job_id < 1 || job_id > job_count) {
        fprintf(stderr, "bg: no such job %d\n", job_id);
        return;
    }

    pid_t pid = jobs[job_id - 1];
    kill(pid, SIGCONT);
}

void kill_job(int job_id) {
    if (job_id < 1 || job_id > job_count) {
        fprintf(stderr, "kill: no such job %d\n", job_id);
        return;
    }

    pid_t pid = jobs[job_id - 1];
    kill(pid, SIGKILL);
    remove_job(pid);
}
