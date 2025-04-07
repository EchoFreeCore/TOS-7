#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

typedef struct {
    int width;
    int long_flag;   // 0 = none, 1 = l, 2 = ll
    int short_flag;  // 0 = none, 1 = h, 2 = hh
    int suppress;
    char specifier;
} format_spec_t;

static void skip_whitespace(const char** str) {
    while (isspace((unsigned char)**str)) (*str)++;
}

static int parse_int(const char** str, int width, int64_t* out) {
    const char* s = *str;
    skip_whitespace(&s);

    int sign = 1;
    if (*s == '-') { sign = -1; s++; width--; }
    else if (*s == '+') { s++; width--; }

    int64_t value = 0;
    int digits = 0;

    while (*s && isdigit((unsigned char)*s) && (width == 0 || digits < width)) {
        int digit = *s - '0';

        if (value > (INT64_MAX - digit) / 10) {
            // Overflow
            return 0;
        }

        value = value * 10 + digit;
        s++; digits++;
    }

    if (digits == 0) return 0;

    *out = sign * value;
    *str = s;
    return 1;
}

static int parse_unsigned(const char** str, int width, int base, uint64_t* out) {
    const char* s = *str;
    skip_whitespace(&s);

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s += 2;
        if (width >= 2) width -= 2;  // anpassa bredden
    }
    else if ((base == 0 || base == 8) && s[0] == '0') {
        base = 8;
        s += 1;
        if (width >= 1) width -= 1;
    }
    else if (base == 0) {
        base = 10;
    }

    uint64_t value = 0;
    int digits = 0;

    while (*s && (width == 0 || digits < width)) {
        char ch = *s;
        int digit = -1;

        if (isdigit(ch)) digit = ch - '0';
        else if (isalpha(ch)) digit = tolower(ch) - 'a' + 10;

        if (digit < 0 || digit >= base) break;

        if (value > (UINT64_MAX - digit) / base) {
            return 0;
        }

        value = value * base + digit;
        s++; digits++;
    }

    if (digits == 0) return 0;

    *out = value;
    *str = s;
    return 1;
}

static int parse_float(const char** str, int width, double* out) {
    const char* s = *str;
    skip_whitespace(&s);

    char buffer[128];
    int len = 0;

    while (*s && len < (int)(sizeof(buffer) - 1) &&
        (width == 0 || len < width) &&
        (isdigit((unsigned char)*s) || *s == '.' || *s == '-' || *s == '+' || *s == 'e' || *s == 'E')) {
        buffer[len++] = *s++;
    }

    if (len == 0) return 0;

    buffer[len] = '\0';
    char* endptr;
    *out = strtod(buffer, &endptr);

    if (endptr == buffer) return 0;

    *str += (endptr - buffer);
    return 1;
}

static int parse_format_spec(const char** fmt, format_spec_t* spec) {
    const char* f = *fmt;
    memset(spec, 0, sizeof(*spec));

    // 1. Suppressions-flagga (*)
    if (*f == '*') {
        spec->suppress = 1;
        f++;
    }

    // 2. Width
    while (isdigit((unsigned char)*f)) {
        spec->width = spec->width * 10 + (*f - '0');
        f++;
    }

    // 3. Length modifier
    if (*f == 'h') {
        f++;
        if (*f == 'h') {
            spec->short_flag = 2; // hh
            f++;
        }
        else {
            spec->short_flag = 1; // h
        }
    }
    else if (*f == 'l') {
        f++;
        if (*f == 'l') {
            spec->long_flag = 2; // ll
            f++;
        }
        else {
            spec->long_flag = 1; // l
        }
    }

    // 4. Specifier
    if (*f == '\0') return 0; // error: format string ends unexpectedly

    spec->specifier = *f++;
    *fmt = f;
    return 1;
}


int vsscanf(const char* input, const char* fmt, ...) {
    va_list args;
    int matches = 0;
    const char* start = input;

    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            if (isspace((unsigned char)*fmt)) {
                skip_whitespace(&input);
                fmt++;
                continue;
            }
            if (*fmt != *input) break;
            fmt++; input++;
            continue;
        }

        fmt++;  // skip '%'

        if (*fmt == '%') {
            if (*input != '%') break;
            fmt++; input++;
            continue;
        }

        format_spec_t spec;
        if (!parse_format_spec(&fmt, &spec)) break;

        // Speciell hantering av %n (måste göras efter parse_format_spec)
        if (spec.specifier == 'n') {
            if (!spec.suppress) {
                int* out = va_arg(args, int*);
                *out = (int)(input - start);
                matches++;
            }
            continue;
        }

        // Hantering av %[...]
        if (spec.specifier == '[') {
            int negate = 0;
            if (*fmt == '^') { negate = 1; fmt++; }

            char set[256] = { 0 };
            while (*fmt && *fmt != ']') {
                unsigned char first = (unsigned char)*fmt++;
                if (*fmt == '-' && fmt[1] && fmt[1] != ']') {
                    fmt++; // skip '-'
                    unsigned char last = (unsigned char)*fmt++;
                    for (unsigned char c = first; c <= last; c++) set[c] = 1;
                }
                else {
                    set[first] = 1;
                }
            }
            if (*fmt == ']') fmt++;

            const char* s = input;
            int len = 0;
            while (*s && (spec.width == 0 || len < spec.width)) {
                unsigned char ch = (unsigned char)*s;
                int match = set[ch];
                if ((match && !negate) || (!match && negate)) {
                    s++; len++;
                }
                else break;
            }

            if (len == 0) goto done;

            if (!spec.suppress) {
                char* out = va_arg(args, char*);
                memcpy(out, input, len);
                out[len] = '\0';
                matches++;
            }
            input += len;
            continue;
        }

        // Hantering av vanliga specifiers
        switch (spec.specifier) {
        case 'd': case 'i': {
            int64_t val;
            if (spec.specifier == 'i') {
                uint64_t uval;
                if (!parse_unsigned(&input, spec.width, 0, &uval)) goto done;
                val = (int64_t)uval;
            }
            else {
                if (!parse_int(&input, spec.width, &val)) goto done;
            }
            if (!spec.suppress) {
                if (spec.short_flag == 2)      *va_arg(args, signed char*) = (signed char)val;
                else if (spec.short_flag == 1) *va_arg(args, short*) = (short)val;
                else if (spec.long_flag == 2)  *va_arg(args, int64_t*) = val;
                else if (spec.long_flag == 1)  *va_arg(args, long*) = (long)val;
                else                           *va_arg(args, int*) = (int)val;
                matches++;
            }
            break;
        }

        case 'u': case 'x': case 'X': case 'o': {
            uint64_t val;
            int base = 10;
            if (spec.specifier == 'x' || spec.specifier == 'X') base = 16;
            else if (spec.specifier == 'o') base = 8;

            if (!parse_unsigned(&input, spec.width, base, &val)) goto done;
            if (!spec.suppress) {
                if (spec.short_flag == 2)      *va_arg(args, unsigned char*) = (unsigned char)val;
                else if (spec.short_flag == 1) *va_arg(args, unsigned short*) = (unsigned short)val;
                else if (spec.long_flag == 2)  *va_arg(args, uint64_t*) = val;
                else if (spec.long_flag == 1)  *va_arg(args, unsigned long*) = (unsigned long)val;
                else                           *va_arg(args, unsigned int*) = (unsigned int)val;
                matches++;
            }
            break;
        }

        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': {
            double val;
            if (!parse_float(&input, spec.width, &val)) goto done;
            if (!spec.suppress) {
                *va_arg(args, double*) = val;
                matches++;
            }
            break;
        }

        case 's': {
            const char* s = input;
            int len = 0;
            skip_whitespace(&s);
            while (s[len] && !isspace((unsigned char)s[len]) && (spec.width == 0 || len < spec.width)) len++;

            if (len == 0) goto done;

            if (!spec.suppress) {
                char* out = va_arg(args, char*);
                int copy_len = (spec.width > 0 && len >= spec.width) ? spec.width - 1 : len;
                memcpy(out, s, copy_len);
                out[copy_len] = '\0';
                matches++;
            }
            input = s + len;
            break;
        }

        case 'c': {
            int width = spec.width ? spec.width : 1;
            if (!spec.suppress) {
                char* out = va_arg(args, char*);
                int i;
                for (i = 0; i < width; i++) {
                    if (*input == '\0') goto done;
                    out[i] = *input++;
                }
                matches++;
            }
            else {
                input += width;
            }
            break;
        }

        case 'p': {
            uint64_t val;
            if (!parse_unsigned(&input, 0, 16, &val)) goto done;
            if (!spec.suppress) {
                *va_arg(args, void**) = (void*)(uintptr_t)val;
                matches++;
            }
            break;
        }

        default:
            goto done;
        }
    }

done:
    va_end(args);
    return matches;
