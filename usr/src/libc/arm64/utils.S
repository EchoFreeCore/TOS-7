// arch/arm64/libm/utils.S
.text

// --------------------------------------------------
// double libm_sqrt(double x)
.global libm_sqrt
.type libm_sqrt, %function
libm_sqrt:
    fsqrt d0, d0
    ret

// --------------------------------------------------
// double libm_fabs(double x)
.global libm_fabs
.type libm_fabs, %function
libm_fabs:
    fabd d0, d0, #0.0
    ret

// Alternativ med bitmask (70-talsstyle):
// fmov x1, d0
// and x1, x1, #0x7FFFFFFFFFFFFFFF
// fmov d0, x1

// --------------------------------------------------
// double libm_ceil(double x)
.global libm_ceil
.type libm_ceil, %function
libm_ceil:
    frintp d0, d0   // round toward +∞
    ret

// --------------------------------------------------
// double libm_floor(double x)
.global libm_floor
.type libm_floor, %function
libm_floor:
    frintm d0, d0   // round toward -∞
    ret

// --------------------------------------------------
// double libm_modf(double x, double *iptr)
.global libm_modf
.type libm_modf, %function
libm_modf:
    fmov d1, d0
    frintz d2, d1       // truncate toward zero
    str d2, [x1]        // store integer part
    fsub d0, d1, d2     // fractional = x - int(x)
    ret

// --------------------------------------------------
// double libm_fmod(double x, double y)
// fmod(x, y) = x - trunc(x/y) * y
.global libm_fmod
.type libm_fmod, %function
libm_fmod:
    fdiv d2, d0, d1     // x / y
    frintz d2, d2       // truncate
    fmul d2, d2, d1     // trunc(x/y) * y
    fsub d0, d0, d2     // x - trunc(x/y)*y
    ret
