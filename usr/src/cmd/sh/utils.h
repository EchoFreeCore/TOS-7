#ifndef UTILS_H
#define UTILS_H

void sigint_handler(int sig);

#endif


// utils.c
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "utils.h"

void sigint_handler(int sig) {
    (void)sig; // undvik unused warning
    write(STDOUT_FILENO, "\n$ ", 3);
}
