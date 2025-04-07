#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

/* POSIX ioctl request codes (du definierar dessa i din kernel ABI) */
#ifndef TIOCGETA
#define TIOCGETA   0x5401
#define TIOCSETA   0x5402
#define TIOCSETAW  0x5403
#define TIOCSETAF  0x5404
#define TCFLSH     0x5405
#define TCXONC     0x5406
#define TCSBRK     0x5407
#endif

int tcgetattr(int fd, struct termios* termios_p) {
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }
    return ioctl(fd, TIOCGETA, termios_p);
}

int tcsetattr(int fd, int actions, const struct termios* termios_p) {
    int request;

    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }

    switch (actions) {
    case TCSANOW:    request = TIOCSETA;  break;
    case TCSADRAIN:  request = TIOCSETAW; break;
    case TCSAFLUSH:  request = TIOCSETAF; break;
    default:
        errno = EINVAL;
        return -1;
    }

    return ioctl(fd, request, termios_p);
}

speed_t cfgetispeed(const struct termios* termios_p) {
    return termios_p ? termios_p->c_ispeed : 0;
}

speed_t cfgetospeed(const struct termios* termios_p) {
    return termios_p ? termios_p->c_ospeed : 0;
}

int cfsetispeed(struct termios* termios_p, speed_t speed) {
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }
    termios_p->c_ispeed = speed;
    return 0;
}

int cfsetospeed(struct termios* termios_p, speed_t speed) {
    if (!termios_p) {
        errno = EINVAL;
        return -1;
    }
    termios_p->c_ospeed = speed;
    return 0;
}

int tcflush(int fd, int queue_selector) {
    return ioctl(fd, TCFLSH, (void*)(long)queue_selector);
}

int tcflow(int fd, int action) {
    return ioctl(fd, TCXONC, (void*)(long)action);
}

int tcdrain(int fd) {
    return ioctl(fd, TCSBRK, (void*)1);  // 1 = drain only
}

int tcsendbreak(int fd, int duration) {
    return ioctl(fd, TCSBRK, (void*)(long)(duration == 0 ? 0 : duration));
}