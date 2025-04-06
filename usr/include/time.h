#ifndef _TIME_H
#define _TIME_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long time_t;

/* Broken-down time structure */
struct tm {
    int tm_sec;   /* seconds [0,60] */
    int tm_min;   /* minutes [0,59] */
    int tm_hour;  /* hours [0,23] */
    int tm_mday;  /* day of month [1,31] */
    int tm_mon;   /* months since Jan [0,11] */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* day of week [0,6] Sunday = 0 */
    int tm_yday;  /* day of year [0,365] */
    int tm_isdst; /* daylight saving time flag */
};

/* POSIX.1-1988 time functions */
time_t time(time_t *t);
double difftime(time_t end, time_t start);
char  *ctime(const time_t *timep);
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
unsigned int sleep(unsigned int seconds);
unsigned int alarm(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_H */
