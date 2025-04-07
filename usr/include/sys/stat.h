#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

/* --- POSIX struct stat --- */
struct stat {
    dev_t     st_dev;     /* device ID */
    ino_t     st_ino;     /* inode number */
    mode_t    st_mode;    /* file mode */
    nlink_t   st_nlink;   /* number of hard links */
    uid_t     st_uid;     /* user ID of owner */
    gid_t     st_gid;     /* group ID of owner */
    dev_t     st_rdev;    /* device ID (if special file) */
    off_t     st_size;    /* total size, in bytes */
    time_t    st_atime;   /* time of last access */
    time_t    st_mtime;   /* time of last modification */
    time_t    st_ctime;   /* time of last status change */
};

/* --- File type bits --- */
#define S_IFMT   0170000 /* bitmask for file type */
#define S_IFIFO  0010000 /* named pipe (FIFO) */
#define S_IFCHR  0020000 /* character device */
#define S_IFDIR  0040000 /* directory */
#define S_IFBLK  0060000 /* block device */
#define S_IFREG  0100000 /* regular file */
#define S_IFLNK  0120000 /* symbolic link (optional POSIX.1-1988) */
#define S_IFSOCK 0140000 /* socket (optional POSIX.1-1988) */

/* --- Special mode flags --- */
#define S_ISUID  0004000 /* set-user-ID bit */
#define S_ISGID  0002000 /* set-group-ID bit */

/* --- User permissions --- */
#define S_IRWXU  00700    /* read, write, execute/search by owner */
#define S_IRUSR  00400    /* read permission, owner */
#define S_IWUSR  00200    /* write permission, owner */
#define S_IXUSR  00100    /* execute/search permission, owner */

/* --- Group permissions --- */
#define S_IRWXG  00070
#define S_IRGRP  00040
#define S_IWGRP  00020
#define S_IXGRP  00010

/* --- Others permissions --- */
#define S_IRWXO  00007
#define S_IROTH  00004
#define S_IWOTH  00002
#define S_IXOTH  00001

/* --- File type testing macros --- */
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

/* --- POSIX.1-1988 system calls --- */
int stat(const char *pathname, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *pathname, struct stat *buf); /* optional but common */
int chmod(const char *path, mode_t mode);
mode_t umask(mode_t mask);
int mkdir(const char *pathname, mode_t mode);

#endif /* _SYS_STAT_H */




