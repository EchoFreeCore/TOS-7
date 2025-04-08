#include <math.h>
#include <errno.h>

/* Deklarera dina assembler-funktioner */
extern double libm_acos(double);
extern double libm_asin(double);
extern double libm_atan(double);
extern double libm_atan2(double, double);
extern double libm_cos(double);
extern double libm_sin(double);
extern double libm_tan(double);
extern double libm_cosh(double);
extern double libm_sinh(double);
extern double libm_tanh(double);
extern double libm_expd(double);
extern double libm_frexp(double, int *);
extern double libm_ldexp(double, int);
extern double libm_log(double);
extern double libm_log10(double);
extern double libm_modf(double, double *);
extern double libm_pow(double, double);
extern double libm_sqrt(double);
extern double libm_ceil(double);
extern double libm_fabs(double);
extern double libm_floor(double);
extern double libm_fmod(double, double);

/* Wrappers (POSIX.1-1988 stil) */

double acos(double x)  { return libm_acos(x); }
double asin(double x)  { return libm_asin(x); }
double atan(double x)  { return libm_atan(x); }
double atan2(double y, double x) { return libm_atan2(y, x); }
double cos(double x)   { return libm_cos(x); }
double sin(double x)   { return libm_sin(x); }
double tan(double x)   { return libm_tan(x); }
double cosh(double x)  { return libm_cosh(x); }
double sinh(double x)  { return libm_sinh(x); }
double tanh(double x)  { return libm_tanh(x); }
double exp(double x)   { return libm_expd(x); }
double frexp(double x, int *exp) { return libm_frexp(x, exp); }
double ldexp(double x, int exp)  { return libm_ldexp(x, exp); }

double log(double x) {
    if (x <= 0.0) {
        errno = EDOM;
        return HUGE_VAL;
    }
    return libm_log(x);
}

double log10(double x) {
    if (x <= 0.0) {
        errno = EDOM;
        return HUGE_VAL;
    }
    return libm_log10(x);
}

double modf(double x, double *iptr) { return libm_modf(x, iptr); }

double pow(double x, double y) {
    if (x == 0.0 && y <= 0.0) {
        errno = EDOM;
        return HUGE_VAL;
    }
    return libm_pow(x, y);
}

double sqrt(double x) {
    if (x < 0.0) {
        errno = EDOM;
        return HUGE_VAL;
    }
    return libm_sqrt(x);
}

double ceil(double x)   { return libm_ceil(x); }
double fabs(double x)   { return libm_fabs(x); }
double floor(double x)  { return libm_floor(x); }
double fmod(double x, double y) {
    if (y == 0.0) {
        errno = EDOM;
        return HUGE_VAL;
    }
    return libm_fmod(x, y);
}