// arch/arm64/libm/hyper.S
// VFP64 hyperbolic functions – sinh, cosh, tanh
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

    // x² / 2
    mov x1, #2
    scvtf d6, x1
    fdiv d2, d2, d6

    // x³ / 6
    mov x1, #6
    scvtf d6, x1
    fdiv d3, d3, d6

    // x⁴ / 24
    mov x1, #24
    scvtf d6, x1
    fdiv d4, d4, d6

    // x⁵ / 120
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
// double libm_sinh(double x) = (exp(x) - exp(-x)) / 2
.global libm_sinh
.type libm_sinh, %function
libm_sinh:
    fmov d1, d0
    bl libm_expd       // exp(x) -> d0
    fmov d2, d0        // store exp(x)

    fneg d1, d1        // -x
    fmov d0, d1
    bl libm_expd       // exp(-x) -> d0

    fsub d0, d2, d0    // exp(x) - exp(-x)
    mov x1, #2
    scvtf d1, x1
    fdiv d0, d0, d1
    ret

// --------------------------------------------------
// double libm_cosh(double x) = (exp(x) + exp(-x)) / 2
.global libm_cosh
.type libm_cosh, %function
libm_cosh:
    fmov d1, d0
    bl libm_expd       // exp(x) -> d0
    fmov d2, d0        // store exp(x)

    fneg d1, d1
    fmov d0, d1
    bl libm_expd       // exp(-x) -> d0

    fadd d0, d2, d0    // exp(x) + exp(-x)
    mov x1, #2
    scvtf d1, x1
    fdiv d0, d0, d1
    ret

// --------------------------------------------------
// double libm_tanh(double x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
.global libm_tanh
.type libm_tanh, %function
libm_tanh:
    fmov d1, d0
    bl libm_expd       // exp(x) -> d0
    fmov d2, d0        // numerator start
    fmov d3, d0        // denominator start

    fneg d1, d1
    fmov d0, d1
    bl libm_expd       // exp(-x) -> d0

    fsub d2, d2, d0    // num = exp(x) - exp(-x)
    fadd d3, d3, d0    // den = exp(x) + exp(-x)
    fdiv d0, d2, d3
    ret
