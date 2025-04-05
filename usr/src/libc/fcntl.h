#ifndef _FCNTL_H
#define _FCNTL_H

/* File access modes and flags for open() */

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_CREAT  0x0200
#define O_EXCL   0x0800
#define O_NOCTTY 0x2000
#define O_TRUNC  0x0400
#define O_APPEND 0x0008
#define O_NONBLOCK 0x0004

#endif /* _FCNTL_H */