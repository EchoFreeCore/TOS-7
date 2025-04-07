#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

static int dtoa_exponential(double val, char *buf, size_t bufsize, int precision, bool uppercase) {
    if (!buf || bufsize == 0) return -1;
    if (precision < 0) precision = 6;
    if (precision > 18) precision = 18;

    char *p = buf;
    size_t len = 0;

    #define CHECK_SPACE(n) do { if ((len + (n)) >= bufsize) return -1; } while(0)

    // Specialfall
    if (isnan(val)) {
        CHECK_SPACE(4);
        buf[0] = 'n'; buf[1] = 'a'; buf[2] = 'n'; buf[3] = '\0';
        return 3;
    }

    if (isinf(val)) {
        CHECK_SPACE(5);
        if (val < 0.0) {
            buf[0] = '-'; buf[1] = 'i'; buf[2] = 'n'; buf[3] = 'f'; buf[4] = '\0';
            return 4;
        } else {
            buf[0] = 'i'; buf[1] = 'n'; buf[2] = 'f'; buf[3] = '\0';
            return 3;
        }
    }

    // Negativt tal
    if (val < 0.0) {
        CHECK_SPACE(1);
        *p++ = '-'; len++;
        val = -val;
    }

    if (val == 0.0) {
        CHECK_SPACE(2 + precision + 5);
        *p++ = '0'; len++;
        *p++ = '.'; len++;
        for (int i = 0; i < precision; ++i) {
            *p++ = '0'; len++;
        }
        *p++ = uppercase ? 'E' : 'e'; len++;
        *p++ = '+'; *p++ = '0'; *p++ = '0'; *p++ = '0'; len += 4;
        *p = '\0';
        return (int)len;
    }

    // Exponent
    int exponent = 0;
    while (val >= 10.0) { val /= 10.0; exponent++; }
    while (val < 1.0)  { val *= 10.0; exponent--; }

    // Avrunda
    double rounding = 0.5;
    for (int i = 0; i < precision; ++i) rounding /= 10.0;
    val += rounding;

    if (val >= 10.0) {
        val /= 10.0;
        exponent++;
    }

    // Ledande siffra
    int digit = (int)val;
    CHECK_SPACE(1);
    *p++ = '0' + digit; len++;
    val -= digit;

    // Punkt
    CHECK_SPACE(1);
    *p++ = '.'; len++;

    // Decimaler
    for (int i = 0; i < precision; ++i) {
        val *= 10.0;
        int d = (int)val;
        CHECK_SPACE(1);
        *p++ = '0' + d; len++;
        val -= d;
    }

    // Exponent-del (e±XYZ)
    CHECK_SPACE(5);
    *p++ = uppercase ? 'E' : 'e'; len++;
    if (exponent >= 0) {
        *p++ = '+'; len++;
    } else {
        *p++ = '-'; len++;
        exponent = -exponent;
    }

    int hundreds = exponent / 100;
    int tens     = (exponent / 10) % 10;
    int ones     = exponent % 10;

    *p++ = '0' + hundreds; len++;
    *p++ = '0' + tens;     len++;
    *p++ = '0' + ones;     len++;

    *p = '\0';
    return (int)len;

    #undef CHECK_SPACE
}


static char *utoa(unsigned long long val, int base, char *end, bool uppercase) {
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    *--end = '\0';  // Nollterminera

    if (val == 0) {
        *--end = '0';
        return end;
    }

    while (val) {
        *--end = digits[val % base];
        val /= base;
    }

    return end;
}

static int ftoa(double val, char *buf, size_t bufsize, int precision) {
    if (precision < 0) precision = 6;
    if (precision > 18) precision = 18;

    if (bufsize == 0) return -1;

    char *p = buf;
    size_t len = 0;

    if (val < 0.0) {
        if (len + 1 >= bufsize) return -1;
        *p++ = '-';
        val = -val;
        len++;
    }

    // Runda talet först
    double rounding = 0.5;
    for (int i = 0; i < precision; ++i) rounding /= 10.0;
    val += rounding;

    double intpart;
    double frac = modf(val, &intpart);
    unsigned long long int_part = (unsigned long long)intpart;

    // Konvertera heltalsdel
    char intbuf[32];
    int i = 0;
    do {
        intbuf[i++] = '0' + (int_part % 10);
        int_part /= 10;
    } while (int_part && i < (int)sizeof(intbuf));

    if (len + i + 1 >= bufsize) return -1;
    while (i--) *p++ = intbuf[i];
    len = (size_t)(p - buf);

    if (precision == 0) {
        *p = '\0';
        return (int)len;
    }

    if (len + 1 >= bufsize) return -1;
    *p++ = '.';
    len++;

    // Skala upp och avrunda fraktionsdelen
    unsigned long long frac_part = (unsigned long long)(frac * pow(10.0, precision));

    // Fyll decimaler
    char fracbuf[32];
    int fi = 0;
    do {
        fracbuf[fi++] = '0' + (frac_part % 10);
        frac_part /= 10;
    } while (fi < precision);

    if (len + fi >= bufsize) return -1;
    while (fi--) *p++ = fracbuf[fi];
    *p = '\0';

    return (int)(p - buf);
}

static void write_with_padding(void (*putc)(char, void *), void *userdata,
                               const char *str, int len, int width, char pad, bool left_justify) {
    if (left_justify) pad = ' ';  // zero-padding ignoreras vid vänsterjustering

    int pad_len = (width > len) ? (width - len) : 0;

    if (!left_justify) {
        for (int i = 0; i < pad_len; ++i)
            putc(pad, userdata);
    }

    for (int i = 0; i < len; ++i)
        putc(str[i], userdata);

    if (left_justify) {
        for (int i = 0; i < pad_len; ++i)
            putc(' ', userdata);
    }
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap) {
    typedef struct {
        char *buf;
        size_t max;
        size_t len;
    } buffer_ctx_t;

    buffer_ctx_t ctx = { .buf = str, .max = size, .len = 0 };

    void buffer_putc(char c, void *userdata) {
        buffer_ctx_t *ctx = (buffer_ctx_t *)userdata;
        if (ctx->len < ctx->max - 1)
            ctx->buf[ctx->len++] = c;
    }

    void (*putc)(char, void *) = buffer_putc;

    while (*fmt) {
        if (*fmt != '%') {
            putc(*fmt++, &ctx);
            continue;
        }

        fmt++;  // skip '%'

        // Flags
        bool left_justify = false, zero_pad = false;
        bool plus_flag = false, alt_form = false;

        while (*fmt == '-' || *fmt == '0' || *fmt == '+' || *fmt == '#' || *fmt == ' ') {
            if (*fmt == '-') left_justify = true;
            if (*fmt == '0') zero_pad = true;
            if (*fmt == '+') plus_flag = true;
            if (*fmt == '#') alt_form = true;
            fmt++;
        }

        // Width
        int width = 0;
        if (*fmt == '*') {
            width = va_arg(ap, int);
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9')
                width = width * 10 + (*fmt++ - '0');
        }

        // Precision
        int precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (*fmt == '*') {
                precision = va_arg(ap, int);
                fmt++;
            } else {
                precision = 0;
                while (*fmt >= '0' && *fmt <= '9')
                    precision = precision * 10 + (*fmt++ - '0');
            }
        }

        // Length
        enum { LEN_NONE, LEN_LONG, LEN_LONGLONG } length = LEN_NONE;
        if (*fmt == 'l') {
            fmt++;
            if (*fmt == 'l') {
                length = LEN_LONGLONG;
                fmt++;
            } else {
                length = LEN_LONG;
            }
        }

        // Format specifier
        char spec = *fmt++;
        char numbuf[128];
        char *numstr;

        #define WRITE_PAD(s, l) \
            write_with_padding(putc, &ctx, s, l, width, zero_pad ? '0' : ' ', left_justify)

        switch (spec) {
            case 'd': {
                long long val = (length == LEN_LONGLONG) ? va_arg(ap, long long)
                               : (length == LEN_LONG) ? va_arg(ap, long)
                               : va_arg(ap, int);
                bool neg = val < 0;
                unsigned long long uval = neg ? (unsigned long long)(-val) : (unsigned long long)val;
                numstr = utoa(uval, 10, numbuf + sizeof(numbuf), false);
                int len = (int)(numbuf + sizeof(numbuf) - 1 - numstr);
                while (precision > len) *--numstr = '0', len++;
                if (neg) *--numstr = '-';
                else if (plus_flag) *--numstr = '+';
                len = (int)(numbuf + sizeof(numbuf) - 1 - numstr);
                WRITE_PAD(numstr, len);
                break;
            }

            case 'u': case 'x': case 'X': case 'o': {
                int base = (spec == 'o') ? 8 : (spec == 'u') ? 10 : 16;
                bool uppercase = (spec == 'X');
                unsigned long long val = (length == LEN_LONGLONG) ? va_arg(ap, unsigned long long)
                                           : (length == LEN_LONG) ? va_arg(ap, unsigned long)
                                           : va_arg(ap, unsigned);
                numstr = utoa(val, base, numbuf + sizeof(numbuf), uppercase);
                int len = (int)(numbuf + sizeof(numbuf) - 1 - numstr);
                while (precision > len) *--numstr = '0', len++;
                if (alt_form && val != 0) {
                    if (base == 8 && *numstr != '0') *--numstr = '0', len++;
                    else if (base == 16) {
                        *--numstr = uppercase ? 'X' : 'x';
                        *--numstr = '0';
                        len += 2;
                    }
                }
                len = (int)(numbuf + sizeof(numbuf) - 1 - numstr);
                WRITE_PAD(numstr, len);
                break;
            }

            case 'f': {
                double dval = va_arg(ap, double);
                int len = ftoa(dval, numbuf, sizeof(numbuf), precision);
                char *out = numbuf;
                if (plus_flag && dval >= 0.0) *--out = '+', len++;
                WRITE_PAD(out, len);
                break;
            }

            case 'e':
            case 'E': {
                double dval = va_arg(ap, double);
                bool uppercase = (spec == 'E');
                int len = dtoa_exponential(dval, numbuf, sizeof(numbuf), precision);
                char *out = numbuf;
                if (plus_flag && dval >= 0.0) *--out = '+', len++;
                WRITE_PAD(out, len);
                break;
            }

case 'g':
case 'G': {
    double dval = va_arg(ap, double);
    bool use_upper = (spec == 'G');
    int prec = (precision < 0) ? 6 : (precision == 0 ? 1 : precision);

    double absval = fabs(dval);
    int exp10 = 0;
    if (absval != 0.0) {
        double norm = absval;
        while (norm >= 10.0) { norm /= 10.0; exp10++; }
        while (norm < 1.0)  { norm *= 10.0; exp10--; }
    }

    char *bufptr = numbuf;
    int len;

    if (exp10 < -4 || exp10 >= prec) {
        len = dtoa_exponential(dval, numbuf, sizeof(numbuf), prec - 1);
        if (use_upper) {
            char *e = strchr(numbuf, 'e');
            if (e) *e = 'E';
        }
    } else {
        len = ftoa(dval, numbuf, sizeof(numbuf), prec);
    }

    // Trim trailing nollor och punkt om !alt_form
    if (!alt_form && len > 0) {
        char *dot = strchr(numbuf, '.');
        if (dot) {
            char *end = numbuf + len;
            while (end > dot && *(end - 1) == '0') --end;
            if (*(end - 1) == '.') --end;
            *end = '\0';
            len = (int)(end - numbuf);
        }
    }

    if (plus_flag && dval >= 0.0) {
        if (bufptr > numbuf) bufptr--;
        else memmove(numbuf + 1, numbuf, len + 1), bufptr = numbuf;
        *bufptr = '+';
        len++;
    }

    write_with_padding(putc, &ctx, bufptr, len, width, zero_pad ? '0' : ' ', left_justify);
    break;
}

            case 's': {
                const char *s = va_arg(ap, const char *);
                int len = (precision >= 0) ? (int)strnlen(s, precision) : (int)strlen(s);
                WRITE_PAD(s, len);
                break;
            }

            case 'c': {
                char c = (char)va_arg(ap, int);
                WRITE_PAD(&c, 1);
                break;
            }

            case 'p': {
                uintptr_t val = (uintptr_t)va_arg(ap, void *);
                numstr = utoa(val, 16, numbuf + sizeof(numbuf), false);
                *--numstr = 'x';
                *--numstr = '0';
                int len = (int)(numbuf + sizeof(numbuf) - 1 - numstr);
                WRITE_PAD(numstr, len);
                break;
            }

            case '%':
                putc('%', &ctx);
                break;

            default:
                putc('%', &ctx);
                putc(spec, &ctx);
                break;
        }

        #undef WRITE_PAD
    }

    if (ctx.len < ctx.max)
        ctx.buf[ctx.len] = '\0';
    else if (ctx.max > 0)
        ctx.buf[ctx.max - 1] = '\0';

    return (int)ctx.len;
}