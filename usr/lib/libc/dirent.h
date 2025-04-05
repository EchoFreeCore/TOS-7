#ifndef _DIRENT_H
#define _DIRENT_H

typedef struct DIR DIR;
struct dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[256];
};

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif // _DIRENT_H