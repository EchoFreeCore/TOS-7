#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <sys/types.h>
typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 8
>>>>>>> 5a6fadfc98e4501f1893c2008cc83e3d49ce0125

#define VINTR    0  /* Interrupt */
#define VQUIT    1  /* Quit */
#define VERASE   2  /* Erase char */
#define VKILL    3  /* Kill line */
#define VEOF     4  /* End-of-file */
#define VMIN     5  /* Min chars for read */
#define VTIME    6  /* Timeout in deciseconds */
#define VSUSP    7  /* Suspend */

struct termios {
    tcflag_t c_iflag;     /* Input mode flags */
    tcflag_t c_oflag;     /* Output mode flags */
    tcflag_t c_cflag;     /* Control mode flags */
    tcflag_t c_lflag;     /* Local mode flags */
    cc_t     c_cc[NCCS];  /* Control characters */
    speed_t  c_ispeed;    /* Input speed */
    speed_t  c_ospeed;    /* Output speed */
};

    /* Input flags */
#define BRKINT   0x0001
#define ICRNL    0x0002
#define IGNBRK   0x0004
#define IGNCR    0x0008
#define INLCR    0x0010
#define INPCK    0x0020
#define ISTRIP   0x0040
#define IXON     0x0080

/* Output flags */
#define OPOST    0x0001

/* Control flags */
#define CS5      0x0000
#define CS6      0x0010
#define CS7      0x0020
#define CS8      0x0030
#define CSIZE    0x0030
#define CSTOPB   0x0040
#define CREAD    0x0080
#define PARENB   0x0100
#define PARODD   0x0200

/* Local flags */
#define ISIG     0x0001
#define ICANON   0x0002
#define ECHO     0x0004
#define ECHOE    0x0008
#define ECHOK    0x0010
#define NOFLSH   0x0020

/* Baud rates */
#define B0       0
#define B50      50
#define B75      75
#define B110     110
#define B134     134
#define B150     150
#define B200     200
#define B300     300
#define B600     600
#define B1200    1200
#define B1800    1800
#define B2400    2400
#define B4800    4800
#define B9600    9600
#define B19200   19200
#define B38400   38400

/* tcsetattr actions */
#define TCSANOW     0
#define TCSADRAIN   1
#define TCSAFLUSH   2

/* tcflush queue selectors */
#define TCIFLUSH    0
#define TCOFLUSH    1
#define TCIOFLUSH   2

/* tcflow actions */
#define TCOOFF      0
#define TCOON       1
#define TCIOFF      2
#define TCION       3

/* POSIX API */
int tcgetattr(int fd, struct termios* termios_p);
int tcsetattr(int fd, int actions, const struct termios* termios_p);
speed_t cfgetispeed(const struct termios* termios_p);
speed_t cfgetospeed(const struct termios* termios_p);
int cfsetispeed(struct termios* termios_p, speed_t speed);
int cfsetospeed(struct termios* termios_p, speed_t speed);
int tcflush(int fd, int queue_selector);
int tcflow(int fd, int action);
int tcdrain(int fd);
int tcsendbreak(int fd, int duration);

#endif /* _TERMIOS_H */
