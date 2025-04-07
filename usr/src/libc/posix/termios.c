#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

/* POSIX ioctl request codes (du definierar dessa i din kernel ABI) */
#ifndef TIOCGETA
#include <sys/ioccom.h>  // Du måste ha detta för _IO* macros

#define TIOCGETA    _IOR('t', 19, struct termios)
#define TIOCSETA    _IOW('t', 20, struct termios)
#define TIOCSETAW   _IOW('t', 21, struct termios)
#define TIOCSETAF   _IOW('t', 22, struct termios)

#define TCSBRK      _IO('t', 123)
#define TCFLSH      _IO('t', 124)
#define TCXONC      _IO('t', 125)
#endif

int tcgetattr(int fd, struct termios *termios_p) {
    if (termios_p == NULL) {
        errno = EINVAL;
        return -1;
    }

    int ret = ioctl(fd, TIOCGETA, termios_p);
    if (ret < 0) {
        if (errno == 0)
            errno = ENOTTY;
        return -1;
    }

    return 0;
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

void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    t->c_oflag &= ~(OPOST);
    t->c_cflag |= (CS8);
    t->c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    t->c_cc[VMIN] = 1;
    t->c_cc[VTIME] = 0;
}