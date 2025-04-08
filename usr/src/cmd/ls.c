#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

int A_option = 0, a_option = 0, f_option = 0, l_option = 0;
int i_option = 0, s_option = 0, t_option = 0, r_option = 0;
int S_option = 0, X_option = 0, one_option = 0;

struct file_entry {
    struct dirent *d;
    time_t mtime;
    off_t size;
};

const char *program_name;

void usage(void) {
    write(2, "usage: ls [-AaflSistXr1] [file...]\n", 35);
    exit(1);
}

void get_mode_str(char *str, mode_t mode) {
    char type = '-';
    if ((mode & S_IFMT) == S_IFDIR) type = 'd';
    else if ((mode & S_IFMT) == S_IFCHR) type = 'c';
    else if ((mode & S_IFMT) == S_IFBLK) type = 'b';
    else if ((mode & S_IFMT) == S_IFIFO) type = 'p';
    else if ((mode & S_IFMT) == S_IFLNK) type = 'l';

    str[0] = type;
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void print_time(time_t t) {
    struct tm *tm = localtime(&t);
    char buf[26];
    if (tm) {
        strcpy(buf, asctime(tm));
        buf[24] = '\0';
        printf(" %.12s", buf + 4);
    }
}

void list_file_short(const char *dir, const char *name) {
    char path[PATH_MAX];
    struct stat st;

    sprintf(path, "%s/%s", dir, name);
    if (stat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (s_option) printf("%4ld ", (long)st.st_blocks);
    if (i_option) printf("%8lu ", (unsigned long)st.st_ino);
    printf("%s", name);
    putchar('\n');
}

void list_file_long(const char *dir, const char *name) {
    char path[PATH_MAX];
    struct stat st;
    char mode_str[11];
    ssize_t len;
    char link_target[PATH_MAX];

    sprintf(path, "%s/%s", dir, name);
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (s_option) printf("%4ld ", (long)st.st_blocks);
    if (i_option) printf("%8lu ", (unsigned long)st.st_ino);

    get_mode_str(mode_str, st.st_mode);
    printf("%s %2ld %5u %5u %6ld",
           mode_str,
           (long)st.st_nlink,
           st.st_uid,
           st.st_gid,
           (long)st.st_size);

    print_time(st.st_mtime);
    printf(" %s", name);

    if ((st.st_mode & S_IFMT) == S_IFLNK) {
        len = readlink(path, link_target, sizeof(link_target) - 1);
        if (len > 0) {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }

    putchar('\n');
}

int skip_dot(const struct dirent *d) {
    return strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0;
}

int skip_hidden(const struct dirent *d) {
    return d->d_name[0] == '.';
}

const char *get_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

int compare_alpha(const void *a, const void *b) {
    const struct file_entry *fa = (const struct file_entry *)a;
    const struct file_entry *fb = (const struct file_entry *)b;
    int cmp = strcmp(fa->d->d_name, fb->d->d_name);
    return r_option ? -cmp : cmp;
}

int compare_mtime(const void *a, const void *b) {
    const struct file_entry *fa = (const struct file_entry *)a;
    const struct file_entry *fb = (const struct file_entry *)b;
    if (fa->mtime > fb->mtime) return r_option ? 1 : -1;
    if (fa->mtime < fb->mtime) return r_option ? -1 : 1;
    return 0;
}

int compare_size(const void *a, const void *b) {
    const struct file_entry *fa = (const struct file_entry *)a;
    const struct file_entry *fb = (const struct file_entry *)b;
    if (fa->size > fb->size) return r_option ? 1 : -1;
    if (fa->size < fb->size) return r_option ? -1 : 1;
    return 0;
}

int compare_ext(const void *a, const void *b) {
    const struct file_entry *fa = (const struct file_entry *)a;
    const struct file_entry *fb = (const struct file_entry *)b;
    const char *ea = get_ext(fa->d->d_name);
    const char *eb = get_ext(fb->d->d_name);
    int cmp = strcmp(ea, eb);
    return r_option ? -cmp : cmp;
}

void list_dir(const char *path,
              int (*skip)(const struct dirent *),
              void (*list)(const char *, const char *)) {
    DIR *dir = opendir(path);
    struct dirent *d;
    struct file_entry *entries = NULL, *tmp;
    size_t count = 0, size = 0;

    if (!dir) {
        perror(path);
        return;
    }

    while ((d = readdir(dir)) != NULL) {
        struct stat st;
        char fullpath[PATH_MAX];
        size_t len;
        struct dirent *copy;

        if (skip && skip(d)) continue;

        if (f_option) {
            list(path, d->d_name);
            continue;
        }

        if (count >= size) {
            size = size ? size * 2 : 64;
            tmp = (struct file_entry *)realloc(entries, size * sizeof(*entries));
            if (!tmp) {
                perror("realloc");
                free(entries);
                closedir(dir);
                return;
            }
            entries = tmp;
        }

        len = sizeof(struct dirent) + strlen(d->d_name) + 1;
        copy = (struct dirent *)malloc(len);
        if (!copy) {
            perror("malloc");
            closedir(dir);
            return;
        }
        memcpy(copy, d, len);
        entries[count].d = copy;

        sprintf(fullpath, "%s/%s", path, d->d_name);
        if (stat(fullpath, &st) == -1) {
            entries[count].mtime = 0;
            entries[count].size = 0;
        } else {
            entries[count].mtime = st.st_mtime;
            entries[count].size = st.st_size;
        }

        count++;
    }

    closedir(dir);

    if (!f_option && entries) {
        int (*cmp_fn)(const void *, const void *);
        size_t i;

        if (S_option)
            cmp_fn = compare_size;
        else if (t_option)
            cmp_fn = compare_mtime;
        else if (X_option)
            cmp_fn = compare_ext;
        else
            cmp_fn = compare_alpha;

        qsort(entries, count, sizeof(*entries), cmp_fn);

        for (i = 0; i < count; i++) {
            list(path, entries[i].d->d_name);
            free(entries[i].d);
        }

        free(entries);
    }
}

int main(int argc, char *argv[]) {
    int i, j, filec;
    char **filev;
    void (*list_fn)(const char *, const char *);
    int (*skip_fn)(const struct dirent *);
    struct stat st;
    static char *default_argv[] = { "." };

    program_name = argv[0];

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-') break;
        for (j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'A': A_option = 1; break;
                case 'a': a_option = 1; break;
                case 'f': f_option = 1; break;
                case 'l': l_option = 1; break;
                case 'i': i_option = 1; break;
                case 's': s_option = 1; break;
                case 't': t_option = 1; break;
                case 'r': r_option = 1; break;
                case 'S': S_option = 1; break;
                case 'X': X_option = 1; break;
                case '1': one_option = 1; break;
                default: usage();
            }
        }
    }

    if (f_option) a_option = 1;
    list_fn = l_option ? list_file_long : list_file_short;

    if (a_option) skip_fn = 0;
    else if (A_option) skip_fn = skip_dot;
    else skip_fn = skip_hidden;

    filec = argc - i;
    filev = argv + i;

    if (filec == 0) {
        filec = 1;
        filev = default_argv;
    }

    for (i = 0; i < filec; i++) {
        if (stat(filev[i], &st) == -1) {
            perror(filev[i]);
            continue;
        }

        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            if (filec > 1) printf("\n%s:\n", filev[i]);
            list_dir(filev[i], skip_fn, list_fn);
        } else {
            list_fn(".", filev[i]);
        }
    }

    return 0;
}
