// arch/arm64/libm/explog.S
.text

// --------------------------------------------------
// double libm_expd(double x)
// exp(x) ≈ 1 + x + x²/2 + x³/6 + x⁴/24 + x⁵/120
.global libm_expd
.type libm_expd, %function
libm_expd:
    fmov d1, d0       // x
    fmul d2, d0, d0   // x²
    fmul d3, d2, d0   // x³
    fmul d4, d3, d0   // x⁴
    fmul d5, d4, d0   // x⁵

    mov x1, #2
    scvtf d6, x1
    fdiv d2, d2, d6

    mov x1, #6
    scvtf d6, x1
    fdiv d3, d3, d6

    mov x1, #24
    scvtf d6, x1
    fdiv d4, d4, d6

    mov x1, #120
    scvtf d6, x1
    fdiv d5, d5, d6

    fmov d0, #1.0
    fadd d0, d0, d1
    fadd d0, d0, d2
    fadd d0, d0, d3
    fadd d0, d0, d4
    fadd d0, d0, d5
    ret

// --------------------------------------------------
// double libm_log(double x)
// log(x) ≈ log2(x) * ln(2)
// log2(x) = exponent + mantissa / 2⁵²
.global libm_log
.type libm_log, %function
libm_log:
    // Reinterpret bits
    mov x1, d0
    lsr x2, x1, #52        // extract exponent
    and x3, x1, #0xFFFFFFFFFFFFF // mantissa
    sub x2, x2, #1023      // unbiased exponent
    scvtf d1, x2           // float(exponent)

    // mantissa / 2⁵²
    ucvtf d2, x3
    ldr d3, =4503599627370496.0  // 2^52
    fdiv d2, d2, d3

    fadd d1, d1, d2        // log2(x)
    ldr d2, =0.6931471805599453 // ln(2)
    fmul d0, d1, d2
    ret

// --------------------------------------------------
// double libm_log10(double x) = log(x) / ln(10)
.global libm_log10
.type libm_log10, %function
libm_log10:
    bl libm_log
    ldr d1, =2.302585092994046 // ln(10)
    fdiv d0, d0, d1
    ret


// --------------------------------------------------
// double libm_pow(double x, double y) = exp(y * log(x))
.global libm_pow
.type libm_pow, %function
libm_pow:
    fmov d2, d0      // x
    fmov d3, d1      // y

    fmov d0, d2
    bl libm_log
    fmul d0, d0, d3
    bl libm_expd
    ret

// --------------------------------------------------
// double libm_ldexp(double x, int exp) = x * 2^exp
.global libm_ldexp
.type libm_ldexp, %function
libm_ldexp:
    scvtf d1, w1           // convert int -> float
    ldr d2, =0.6931471805599453 // ln(2)
    fmul d1, d1, d2        // exp * ln(2)
    fmov d2, d1
    bl libm_expd           // 2^exp
    fmul d0, d0, d2        // x * 2^exp
    ret

// --------------------------------------------------
// double libm_frexp(double x, int *exp)
// returns mantissa in [0.5, 1.0), sets *exp
.global libm_frexp
.type libm_frexp, %function
libm_frexp:
    mov x1, x0             // save pointer to exp
    fmov x2, d0            // reinterpret bits
    and x3, x2, #0x800FFFFFFFFFFFFF  // clear exponent
    orr x3, x3, #(1023 << 52)        // set exponent to 1023 (bias)
    fmov d0, x3

    lsr x4, x2, #52
    and x4, x4, #0x7FF
    sub x4, x4, #1022       // unbiased -1
    str w4, [x1]
    ret
