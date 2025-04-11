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

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAXPATH PATH_MAX

/* Command-line flags */
static int A_flag = 0, a_flag = 0, f_flag = 0, l_flag = 0;
static int i_flag = 0, s_flag = 0, t_flag = 0, r_flag = 0;
static int S_flag = 0, X_flag = 0, one_flag = 0;
static int R_flag = 0; /* Recursive listing */

/* File entry structure */
struct file_entry {
    char name[MAXPATH];
    time_t mtime;
    off_t size;
    ino_t inode;
    blkcnt_t blocks;
};

/* Display usage information and exit */
static void usage(void) {
    fputs("usage: ls [-AaflSistXr1R] [file...]\n", stderr);
    exit(1);
}

/* Convert file mode to permission string */
static void get_mode_string(char *str, mode_t mode) {
    str[0] = S_ISDIR(mode)  ? 'd' :
             S_ISCHR(mode)  ? 'c' :
             S_ISBLK(mode)  ? 'b' :
             S_ISFIFO(mode) ? 'p' :
             S_ISLNK(mode)  ? 'l' :
             S_ISSOCK(mode) ? 's' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_ISUID) ? ((mode & S_IXUSR) ? 's' : 'S') :
                                ((mode & S_IXUSR) ? 'x' : '-');
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_ISGID) ? ((mode & S_IXGRP) ? 's' : 'S') :
                                ((mode & S_IXGRP) ? 'x' : '-');
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_ISVTX) ? ((mode & S_IXOTH) ? 't' : 'T') :
                                ((mode & S_IXOTH) ? 'x' : '-');
    str[10] = '\0';
}

/* Print formatted time */
static void print_time(time_t t) {
    struct tm *tm = localtime(&t);
    char buf[32];
    time_t now = time(NULL);
    if (difftime(now, t) > 15552000 || difftime(t, now) > 15552000)
        strftime(buf, sizeof(buf), "%b %e  %Y", tm);
    else
        strftime(buf, sizeof(buf), "%b %e %H:%M", tm);
    printf(" %s", buf);
}

/* Get file extension (helper for -X) */
static const char *get_ext(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot || dot == name) return "";
    return dot + 1;
}

/* Compare by extension */
static int compare_ext(const void *a, const void *b) {
    const struct file_entry *fa = a, *fb = b;
    int cmp = strcmp(get_ext(fa->name), get_ext(fb->name));
    if (cmp == 0) return compare_name(a, b);
    return r_flag ? -cmp : cmp;
}

/* Compare by name (default) */
static int compare_name(const void *a, const void *b) {
    const struct file_entry *fa = a, *fb = b;
    int cmp = strcmp(fa->name, fb->name);
    return r_flag ? -cmp : cmp;
}

/* Compare by mtime (-t) */
static int compare_mtime(const void *a, const void *b) {
    const struct file_entry *fa = a, *fb = b;
    if (fa->mtime == fb->mtime) return compare_name(a, b);
    return r_flag ? (fa->mtime > fb->mtime) - (fa->mtime < fb->mtime)
                  : (fb->mtime > fa->mtime) - (fb->mtime < fa->mtime);
}

/* Compare by size (-S) */
static int compare_size(const void *a, const void *b) {
    const struct file_entry *fa = a, *fb = b;
    if (fa->size == fb->size) return compare_name(a, b);
    return r_flag ? (fa->size > fb->size) - (fa->size < fb->size)
                  : (fb->size > fa->size) - (fb->size < fa->size);
}

/* List a single file entry */
static void list_entry(const char *dir, struct file_entry *fe) {
    char full[MAXPATH], mode[11];
    struct stat st;
    snprintf(full, sizeof(full), "%s/%s", dir, fe->name);
    if (lstat(full, &st) == -1) {
        fprintf(stderr, "%s: %s\n", full, strerror(errno));
        return;
    }

    if (l_flag) {
        get_mode_string(mode, st.st_mode);
        if (s_flag) printf("%4ld ", (long)fe->blocks);
        if (i_flag) printf("%8lu ", (unsigned long)fe->inode);
        printf("%s %2ld %-8u %-8u %6ld", mode,
               (long)st.st_nlink, st.st_uid, st.st_gid, (long)st.st_size);
        print_time(st.st_mtime);
        printf(" %s\n", fe->name);
    } else {
        if (s_flag) printf("%4ld ", (long)fe->blocks);
        if (i_flag) printf("%8lu ", (unsigned long)fe->inode);
        printf("%s\n", fe->name);
    }
}

/* List contents of directory */
static void list_dir(const char *path) {
    DIR *dp;
    struct dirent *entry;
    struct stat st;
    struct file_entry *list;
    char **subdirs = NULL;
    size_t count = 0, alloc = 64, i, dircount = 0;
    int (*cmp)(const void *, const void *);

    list = malloc(sizeof(*list) * alloc);
    if (!list) {
        perror("malloc");
        return;
    }

    dp = opendir(path);
    if (!dp) {
        perror(path);
        free(list);
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (!a_flag && entry->d_name[0] == '.') continue;
        if (A_flag && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))) continue;

        if (count >= alloc) {
            alloc *= 2;
            void *tmp = realloc(list, sizeof(*list) * alloc);
            if (!tmp) {
                perror("realloc");
                free(list);
                closedir(dp);
                return;
            }
            list = tmp;
        }

        snprintf(list[count].name, sizeof(list[count].name), "%s", entry->d_name);

        char full[MAXPATH];
        snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
        if (lstat(full, &st) == -1) {
            list[count].mtime = 0;
            list[count].size = 0;
            list[count].inode = 0;
            list[count].blocks = 0;
        } else {
            list[count].mtime = st.st_mtime;
            list[count].size = st.st_size;
            list[count].inode = st.st_ino;
            list[count].blocks = st.st_blocks;

            if (R_flag && S_ISDIR(st.st_mode) &&
                strcmp(entry->d_name, ".") != 0 &&
                strcmp(entry->d_name, "..") != 0) {
                char *dup = strdup(full);
                if (!dup) {
                    perror("strdup");
                    continue;
                }
                char **tmp = realloc(subdirs, sizeof(char*) * (dircount + 1));
                if (!tmp) {
                    perror("realloc");
                    free(dup);
                    continue;
                }
                subdirs = tmp;
                subdirs[dircount++] = dup;
            }
        }

        count++;
    }

    closedir(dp);

    if (l_flag || s_flag) {
        blkcnt_t total = 0;
        for (i = 0; i < count; i++) total += list[i].blocks;
        printf("total %ld\n", (long)total);
    }

    if (!f_flag) {
        cmp = compare_name;
        if (X_flag) cmp = compare_ext;
        else if (S_flag) cmp = compare_size;
        else if (t_flag) cmp = compare_mtime;
        qsort(list, count, sizeof(*list), cmp);
    }

    for (i = 0; i < count; i++)
        list_entry(path, &list[i]);

    free(list);

    for (i = 0; i < dircount; i++) {
        printf("\n%s:\n", subdirs[i]);
        list_dir(subdirs[i]);
        free(subdirs[i]);
    }
    free(subdirs);
}