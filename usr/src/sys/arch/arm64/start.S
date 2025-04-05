    .equ KZERO,         0xffff000000000000
    .equ SCTLR_MMU,     0x1                // Bit 0 = MMU enable

    // MAIR_EL1 indexar (nGnRnE = strongly ordered, WB = normal cacheable etc.)
    .equ MA_DEV_nGnRnE, 0
    .equ MA_DEV_nGnRE,  1
    .equ MA_DEV_nGRE,   2
    .equ MA_DEV_GRE,    3
    .equ MA_MEM_UC,     4
    .equ MA_MEM_WT,     5
    .equ MA_MEM_WB,     6

    .equ MAIR_INIT,     (0xFF << (MA_MEM_WB * 8)) | \
                        (0x33 << (MA_MEM_WT * 8)) | \
                        (0x44 << (MA_MEM_UC * 8)) | \
                        (0x00 << (MA_DEV_nGnRnE * 8)) | \
                        (0x04 << (MA_DEV_nGnRE * 8)) | \
                        (0x08 << (MA_DEV_nGRE * 8)) | \
                        (0x0C << (MA_DEV_GRE * 8))

    // Translation setup
    .equ SHARE_INNER,   3
    .equ CACHE_WB,      1
    .equ PGSHIFT,       12
    .equ EVASHIFT,      39

    .equ TCR_INIT, \
        (0 << 38) | (0 << 37) | (0 << 36) | \
        ((((3 << 16) | (1 << 14) | (2 << 12)) >> PGSHIFT) & 3) << 30 | \
        (SHARE_INNER << 28) | (CACHE_WB << 26) | (CACHE_WB << 24) | \
        (0 << 23) | (0 << 22) | ((64 - EVASHIFT) << 16) | \
        ((((1 << 16) | (2 << 14) | (0 << 12)) >> PGSHIFT) & 3) << 14 | \
        (SHARE_INNER << 12) | (CACHE_WB << 10) | (CACHE_WB << 8) | \
        (0 << 7) | ((64 - EVASHIFT) << 0)

    .equ SCTLR_MMU, (1 << 12) | (1 << 2) | (1 << 0)    // I | C | M

    .equ SCTLR_CLR, \
        (3 << 30) | (1 << 27) | (1 << 26) | (1 << 25) | \
        (1 << 24) | (1 << 21) | (1 << 19) | (1 << 18) | \
        (1 << 17) | (1 << 16) | (1 << 15) | (1 << 14) | \
        (1 << 13) | (1 << 10) | (1 << 9)  | (1 << 4)  | \
        (1 << 3)  | (1 << 1)

    .equ SCTLR_SET, \
        (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)






    .text
    .global _start
    .type _start, %function

_start:
    // Spara ev. bootvärde
    mov     x26, x0

    // Sätt static base
    ldr     x28, =setSB - KZERO

    bl      svcmode

    // Använd SP_EL1
    mov     w1, #1
    msr     SPSel, x1

    // Stäng av MMU
    bl      mmudisable

    // Invalidera cache
    bl      cachedwbinv
    bl      l2cacheuwbinv
    bl      cacheiinv

    // Initiera Mach-struktur för aktuell CPU
    ldr     x27, =MACHADDR(0) - KZERO

    mrs     x1, MPIDR_EL1
    and     w1, w1, #(MAXMACH - 1)
    mov     w2, #MACHSIZE
    mul     w2, w1, w2
    sub     x27, x27, x2

    add     x2, x27, #(MACHSIZE - 16)
    mov     sp, x2

    cbnz    w1, _startup       // andra CPU:er går direkt till MMU + wait

    // Init core 0: nollställ page tables
    ldr     x1, =L1BOT - KZERO
    ldr     x2, =MACHADDR(-1) - KZERO
_zerol1:
    str     xzr, [x1], #8
    cmp     x1, x2
    b.ne    _zerol1

    // Nollställ BSS
    ldr     x1, =edata - KZERO
    ldr     x2, =end - KZERO
_zerobss:
    str     xzr, [x1], #8
    cmp     x1, x2
    b.ne    _zerobss

    // Setup sidtabeller
    ldr     x0, =L1BOT - KZERO
    bl      mmuidmap

    ldr     x0, =L1 - KZERO
    bl      mmu0init

    sevl
_startup:
    wfe

    bl      mmuenable

    // Återställ indata till R0, nolla R26
    mov     x0, x26
    mov     x26, #0

    // Sätt TLS-basen (TPIDR_EL1) till Mach + KZERO
    orr     x27, x27, #KZERO
    msr     TPIDR_EL1, x27

    // Ladda static base igen (nu virtuell)
    ldr     x28, =setSB

    bl      main


    .text
    .global stop
    .type stop, %function

stop:
_stop:
    wfe
    b _stop


    .text
    .global aaa
    .type aaa, %function

aaa:
xxx:
    ldr     x1, =0x860040 + VIRTIO  // memory-mapped debug port
    mov     w2, #'A'                // ASCII 'A'
    str     w2, [x1]                // write to port
    b       xxx


    .text
    .global sev
    .type sev, %function

sev:
    sev         // send event
    wfe         // wait for event
    ret



    .text
    .global svcmode
    .type svcmode, %function

svcmode:
    // Maskera IRQ, FIQ, SError, Debug
    msr     DAIFSet, #0xF

    // Läs vilket exception level vi är i
    mrs     x0, CurrentEL
    and     w0, w0, #(3 << 2)        // plocka ut EL bits (bits [3:2])

    cmp     w0, #(1 << 2)            // EL1?
    beq     1f
    cmp     w0, #(2 << 2)            // EL2?
    beq     2f

    b       stop                    // unknown level → stop

// --- EL1 case
1:
    ret

// --- EL2 case: konfigurera EL2 och hoppa till EL1
2:
    // MDCR_EL2 = 0
    mov     x0, #0
    msr     MDCR_EL2, x0
    isb     sy

    // CNTVOFF_EL2 = 0 (ingen offset i virtuell timer)
    mov     x0, #0
    msr     CNTVOFF_EL2, x0

    // HCR_EL2 = RW (bit31), HCD (bit29), SWIO (bit2), BSU (0), FB (0)
    mov     w0, #(1 << 31 | 1 << 29 | 1 << 2)
    msr     HCR_EL2, x0
    isb     sy

    // SCTLR_EL2 = RES1-bitar
    mov     w0, ((3 << 4) | (1 << 11) | (1 << 16) | (1 << 18) | (3 << 22) | (3 << 28))
    isb     sy
    msr     SCTLR_EL2, x0
    isb     sy

    // VTTBR_EL2 = 0 (ingen gäst-VM)
    mov     x0, #0
    msr     VTTBR_EL2, x0
    isb     sy

    // SPSR_EL2 = Masked DAIF + EL1h (0xF << 6 | 0b0100 = 0x104)
    mov     w0, #(0xF << 6 | 0x4)
    msr     SPSR_EL2, x0

    // ELR_EL2 = LR
    msr     ELR_EL2, x30

    // Övergång till EL1
    eret




    .text
    .global mmudisable
    .type mmudisable, %function

mmudisable:
    isb     sy

    mrs     x0, SCTLR_EL1

    // Rensa MMU, I$, D$, och massa andra bitar
    bic     x0, x0, #(SCTLR_CLR | SCTLR_MMU)

    // Sätt bitar som måste vara RES1
    orr     x0, x0, #SCTLR_SET

    isb     sy
    msr     SCTLR_EL1, x0
    isb     sy

    b       flushlocaltlb



    .text
    .global mmuenable
    .type mmuenable, %function

mmuenable:
    // --------------------------------------------------------------------
    // Returnera till virtuell adressrymd efter MMU aktivering
    // LR := LR | KZERO
    // --------------------------------------------------------------------
    orr     x30, x30, #KZERO
    str     x30, [sp, #-16]!       // push LR på stacken

    bl      flushlocaltlb          // töm TLB för lokal CPU

    // --------------------------------------------------------------------
    // MAIR_EL1 setup: Memory Attribute Indirection Register
    // --------------------------------------------------------------------
    ldr     x1, =MAIR_INIT
    msr     MAIR_EL1, x1
    isb     sy

    // --------------------------------------------------------------------
    // TCR_EL1 setup: Translation Control Register
    // --------------------------------------------------------------------
    ldr     x1, =TCR_INIT
    mrs     x2, ID_AA64MMFR0_EL1
    and     x2, x2, #0x7            // PARange[2:0]
    lsl     x2, x2, #32             // shift to [34:32]
    orr     x1, x1, x2
    msr     TCR_EL1, x1
    isb     sy

    // --------------------------------------------------------------------
    // Ladda sidtabeller i TTBR0/TTBR1
    // --------------------------------------------------------------------
    ldr     x0, =L1BOT - KZERO
    ldr     x1, =L1TOP - KZERO
    isb     sy
    msr     TTBR0_EL1, x0
    msr     TTBR1_EL1, x1
    isb     sy

    // --------------------------------------------------------------------
    // Aktivera MMU och cache via SCTLR_EL1
    // --------------------------------------------------------------------
    mrs     x1, SCTLR_EL1
    orr     x1, x1, #SCTLR_MMU
    isb     sy
    msr     SCTLR_EL1, x1
    isb     sy

    // --------------------------------------------------------------------
    // Justera SP till virtuell offset (KZERO)
    // --------------------------------------------------------------------
    mov     x1, sp
    orr     x1, x1, #KZERO
    mov     sp, x1

    // Poppa LR och fortsätt
    ldr     x30, [sp], #16
    b       cacheiinv



    .text
    .global touser
    .type touser, %function

touser:
    mov     w1, #0x10028        // entry point (EL0)
    mov     w2, #0              // psr = mask, no flags

    msr     SP_EL0, x0          // set userland stack
    msr     ELR_EL1, x1         // set PC
    msr     SPSR_EL1, x2        // set PSR (processor state)
    eret                         // enter EL0


    .text
    .global cas
    .global cmpswap
    .type cas, %function
cas:
cmpswap:
    ldr     w1, [sp, #8]     // w1 = ov
    ldr     w2, [sp, #16]    // w2 = nv
1:
    ldaxr   w3, [x0]         // w3 = *addr, acquire exclusive
    cmp     w3, w1
    b.ne    2f               // != ov → miss
    stxr    w4, w2, [x0]     // försöker skriva nv → addr
    cbnz    w4, 1b           // om misslyckad, försök igen
    mov     w0, #1
    dmb     ish              // synka
    ret
2:
    clrex                    // släpp exclusivity
    mov     w0, #0
    ret


    .text
    .global tas
    .global _tas
    .type tas, %function
tas:
_tas:
    mov     w2, #0xdeaddead
1:
    ldaxr   w1, [x0]         // read old value
    stxr    w3, w2, [x0]     // försök skriva in tag-värde
    cbnz    w3, 1b           // om misslyckad, försök igen
    mov     w0, w1           // returnera gammalt värde
    ret


    .text
    .global coherence
    .type coherence, %function

coherence:
    dmb     ish
    ret



    .text
    .global islo
    .type islo, %function

islo:
    mrs     x0, DAIF
    and     x0, x0, #(0x2 << 6)    // maska ut IRQ-bit (bit 7)
    eor     x0, x0, #(0x2 << 6)    // om biten var 1 → 0, annars → 1
    ret

    .text
    .global splhi
    .type splhi, %function

splhi:
    mrs     x0, DAIF
    msr     DAIFSet, #0x2         // set IRQ mask
    ret

    .text
    .global splfhi
    .type splfhi, %function

splfhi:
    mrs     x0, DAIF
    msr     DAIFSet, #0x3         // set FIQ + IRQ mask
    ret


    .text
    .global spllo
    .type spllo, %function

spllo:
    msr     DAIFClr, #0x3         // clear FIQ + IRQ mask
    ret


    .text
    .global splflo
    .type splflo, %function

splflo:
    msr     DAIFClr, #0x1         // clear FIQ only
    ret


    .text
    .global splx
    .type splx, %function

splx:
    msr     DAIF, x0
    ret


    .text
    .global idlehands
    .type idlehands, %function

idlehands:
    dmb     ish                     // data memory barrier (inner-shareable)

    ldr     w0, =nrdy               // ladda adressen till nrdy
    ldr     w0, [x0]                // w0 = *nrdy

    cbnz    w0, 1f                  // om != 0, gå till _ready
    wfi                             // annars: vänta på avbrott
1:
    ret


    .text
    .global vcycles
    .type vcycles, %function

vcycles:
    mrs     x0, CNTVCT_EL0         // virtual counter (monotonic)
    ret

    .text
    .global lcycles
    .type lcycles, %function

lcycles:
    mrs     x0, PMCCNTR_EL0        // performance counter (real cycles)
    ret

    .text
    .global setlabel
    .type setlabel, %function

setlabel:
    str     x30, [x0, #8]   // spara LR (ret addr)
    mov     x1, sp
    str     x1, [x0, #0]    // spara SP
    mov     w0, #0          // return 0
    ret

    .text
    .global gotolabel
    .type gotolabel, %function

gotolabel:
    ldr     x30, [x0, #8]   // ladda LR
    ldr     x1, [x0, #0]    // ladda SP
    mov     sp, x1          // återställ SP
    mov     w0, #1          // return 1
    ret


    .text
    .global returnto
    .type returnto, %function

returnto:
    str     x0, [sp]        // skriv till *SP
    ret


    .text
    .global getfar
    .type getfar, %function

getfar:
    mrs     x0, FAR_EL1
    ret


    .text
    .global setttbr
    .type setttbr, %function

setttbr:
    dsb     ishst
    msr     TTBR0_EL1, x0
    dsb     ish
    isb     sy
    ret

    .text
    .global flushasidva
    .type flushasidva, %function
flushasidva:
tlbivae1is:
    dsb     ishst
    tlbi    vae1is, x0       // x0 = VA + ASID
    dsb     ish
    isb     sy
    ret

    .text
    .global flushasidvall
    .type flushasidvall, %function
flushasidvall:
tlbivale1is:
    dsb     ishst
    tlbi    vale1is, x0      // x0 = VA (applies to all ASIDs)
    dsb     ish
    isb     sy
    ret

    .text
    .global flushasid
    .type flushasid, %function
flushasid:
tlbiaside1is:
    dsb     ishst
    tlbi    aside1is, x0     // x0 = ASID
    dsb     ish
    isb     sy
    ret


    .text
    .global flushtlb
    .type flushtlb, %function
flushtlb:
tlbivmalle1is:
    dsb     ishst
    tlbi    vmalle1is        // full TLB invalidate (EL1, Inner Shareable)
    dsb     ish
    isb     sy
    ret


    .text
    .global flushlocaltlb
    .type flushlocaltlb, %function

flushlocaltlb:
tlbivmalle1:
    // Data Sync Barrier (store-only, non-shareable)
    dsb     nshst

    // Invalidera hela TLB för EL1 på denna CPU
    tlbi    vmalle1

    // Full Data Barrier (non-shareable)
    dsb     nsh

    // Instruktionsbarriär – säkerställ effekt före kod fortsätter
    isb     sy
    ret


    .text
    .global fpon
    .type fpon, %function

fpon:
    // Aktivera FP/SIMD för EL0/EL1 (bits 21:20 = 11)
    mov     w0, #(3 << 20)

    // Skriv till Coprocessor Access Control Register
    msr     CPACR_EL1, x0

    // Instruktionsbarriär: säkerställ att ändringen träder i kraft
    isb     sy
    ret

    .text
    .global fpoff
    .type fpoff, %function

fpoff:
    // x0 = 0, dvs nollställ access för FP/SIMD
    mov     w0, #0

    // Skriv till CPACR_EL1
    msr     CPACR_EL1, x0

    // Instruktionssynk – säkerställ att ändringen gäller innan kod fortsätter
    isb     sy
    ret



    .text
    .global fpsaveregs
    .type fpsaveregs, %function

fpsaveregs:
    st1     {v0.4s,  v1.4s,  v2.4s,  v3.4s},   [x0], #64
    st1     {v4.4s,  v5.4s,  v6.4s,  v7.4s},   [x0], #64
    st1     {v8.4s,  v9.4s,  v10.4s, v11.4s},  [x0], #64
    st1     {v12.4s, v13.4s, v14.4s, v15.4s},  [x0], #64
    st1     {v16.4s, v17.4s, v18.4s, v19.4s},  [x0], #64
    st1     {v20.4s, v21.4s, v22.4s, v23.4s},  [x0], #64
    st1     {v24.4s, v25.4s, v26.4s, v27.4s},  [x0], #64
    st1     {v28.4s, v29.4s, v30.4s, v31.4s},  [x0], #64
    ret

    .text
    .global fploadregs
    .type fploadregs, %function

fploadregs:
    ld1     {v0.4s,  v1.4s,  v2.4s,  v3.4s},   [x0], #64
    ld1     {v4.4s,  v5.4s,  v6.4s,  v7.4s},   [x0], #64
    ld1     {v8.4s,  v9.4s,  v10.4s, v11.4s},  [x0], #64
    ld1     {v12.4s, v13.4s, v14.4s, v15.4s},  [x0], #64
    ld1     {v16.4s, v17.4s, v18.4s, v19.4s},  [x0], #64
    ld1     {v20.4s, v21.4s, v22.4s, v23.4s},  [x0], #64
    ld1     {v24.4s, v25.4s, v26.4s, v27.4s},  [x0], #64
    ld1     {v28.4s, v29.4s, v30.4s, v31.4s},  [x0], #64
    ret


    .text
    .global vsys0
    .type vsys0, %function

vsys0:
    // ESR_EL1 ligger redan i x0 – plocka ut bits [31:26] => Exception Class
    lsr     x17, x0, #26         // x17 = x0 >> 26

    // Är det en SVC från EL0? EC == 0x15
    cmp     w17, #0x15
    b.ne    _itsatrap            // nope → vanliga trapregistret

    // Spara x26–x29
    stp     x26, x27, [sp, #224]
    stp     x28, x29, [sp, #240]

    // Läs state från EL1
    mrs     x1, SP_EL0
    mrs     x2, ELR_EL1
    mrs     x3, SPSR_EL1

    // Spara till trapframe
    str     x0, [sp, #288]   // type = ESR
    str     x1, [sp, #264]   // sp
    str     x2, [sp, #272]   // pc
    str     x3, [sp, #280]   // psr

    // Ladda static base
    adr     x28, setSB

    // TLS & Mach pointer
    mrs     x27, TPIDR_EL1
    ldr     x26, [x27, #16]

    // x0 = ureg
    add     x0, sp, #16

    // Anropa syscall(ureg)
    bl      syscall




    .text
    .global forkret
    .type forkret, %function

forkret:
    // Stäng av IRQ + FIQ
    msr     DAIFSet, #0x3

    // x0 = ureg (pekare till trapframe, används kanske senare i C)
    add     x0, sp, #16

    // Returvärde till barnprocess (läggs i x0 igen)
    ldr     x0, [sp, #16]

    // Återställ stack, pc, psr
    ldr     x1, [sp, #264]     // SP_EL0
    ldr     x2, [sp, #272]     // ELR_EL1
    ldr     x3, [sp, #280]     // SPSR_EL1

    msr     SP_EL0, x1
    msr     ELR_EL1, x2
    msr     SPSR_EL1, x3

    // Återställ callee-saved registers
    ldp     x26, x27, [sp, #224]
    ldp     x28, x29, [sp, #240]

    // Återställ link register (x30)
    ldr     x30, [sp, #256]

    // Frigör trapframe
    add     sp, sp, #TRAPFRAMESIZE

    // Hoppa tillbaka till EL0
    eret


    .text
    .global itsatrap
    .type itsatrap, %function

itsatrap:
_itsatrap:
    stp     x1,  x2,  [sp, #24]
    stp     x3,  x4,  [sp, #40]
    stp     x5,  x6,  [sp, #56]
    stp     x7,  x8,  [sp, #72]
    stp     x9,  x10, [sp, #88]
    stp     x11, x12, [sp, #104]
    stp     x13, x14, [sp, #120]
    stp     x15, x16, [sp, #136]

    // (x17 ignoreras – ABI-skäl)
    stp     x18, x19, [sp, #160]
    stp     x20, x21, [sp, #176]
    stp     x22, x23, [sp, #192]
    stp     x24, x25, [sp, #208]

    .global vtrap0
    .type vtrap0, %function

vtrap0:
    stp     x26, x27, [sp, #224]
    stp     x28, x29, [sp, #240]

    mrs     x1, SP_EL0
    mrs     x2, ELR_EL1
    mrs     x3, SPSR_EL1

    str     x0, [sp, #288]   // type
    str     x1, [sp, #264]   // sp
    str     x2, [sp, #272]   // pc
    str     x3, [sp, #280]   // psr

    adr     x28, setSB       // load static base
    mrs     x27, TPIDR_EL1   // TLS base
    ldr     x26, [x27, #16]  // M = *(TPIDR_EL1 + 16)

    add     x0, sp, #16      // x0 = &ureg
    bl      trap             // trap(ureg)


    .text
    .global noteret
    .type noteret, %function

noteret:
    // Maskera IRQ + FIQ
    msr     DAIFSet, #0x3

    // x0 = pekare till ureg
    add     x0, sp, #16

    // Återställ SP, PC, PSR
    ldr     x1, [sp, #264]    // sp
    ldr     x2, [sp, #272]    // pc
    ldr     x3, [sp, #280]    // psr

    msr     SP_EL0, x1
    msr     ELR_EL1, x2
    msr     SPSR_EL1, x3

    // Återställ x26–x29 från trapframe
    ldp     x26, x27, [sp, #224]
    ldp     x28, x29, [sp, #240]

_intrreturn:
    // Återställ alla vanliga register x0–x25
    ldp     x0,  x1,  [sp, #16]
    ldp     x2,  x3,  [sp, #32]
    ldp     x4,  x5,  [sp, #48]
    ldp     x6,  x7,  [sp, #64]
    ldp     x8,  x9,  [sp, #80]
    ldp     x10, x11, [sp, #96]
    ldp     x12, x13, [sp, #112]
    ldp     x14, x15, [sp, #128]
    ldp     x16, x17, [sp, #144]
    ldp     x18, x19, [sp, #160]
    ldp     x20, x21, [sp, #176]
    ldp     x22, x23, [sp, #192]
    ldp     x24, x25, [sp, #208]

    // Återställ link register (x30)
    ldr     x30, [sp, #256]

    // Frigör trapframe
    add     sp, sp, #TRAPFRAMESIZE

    // Gå tillbaka från undantag
    eret


    .text
    .global vtrap1
    .type vtrap1, %function

vtrap1:
    // Spara "special" register (x29) i offset 248
    str     x29, [sp, #248]

    // x1 = sp + TRAPFRAMESIZE (för att beräkna ureg-position)
    add     x1, sp, #TRAPFRAMESIZE

    // Läs program counter och status
    mrs     x2, elr_el1
    mrs     x3, spsr_el1

    // Spara trap-typ (x0), ureg-sp (x1), pc (x2), psr (x3)
    str     x0, [sp, #288]   // type
    str     x1, [sp, #264]   // ureg sp
    str     x2, [sp, #272]   // pc
    str     x3, [sp, #280]   // psr

    // x0 = &ureg (på stack: sp + 16)
    add     x0, sp, #16

    // Anropa trap()
    bl      trap

    // Disable interrupts (DAIFSet = 0x3 = mask IRQ + FIQ)
    msr     DAIFSet, #0x3

    // Ladda tillbaka pc och psr
    ldr     x2, [sp, #272]   // pc
    ldr     x3, [sp, #280]   // psr

    msr     elr_el1, x2
    msr     spsr_el1, x3

    // Återställ specialregister
    ldr     x29, [sp, #248]

    b       _intrreturn
    

    .text
    .global vsys
    .type vsys, %function

vsys:
    // Allokera trapframe
    sub     sp, sp, #TRAPFRAMESIZE

    // Spara R0 (x0) i offset 16
    str     x0, [sp, #16]

    // Spara link register (x30) i offset 256
    str     x30, [sp, #256]

    // Spara x17 (temporär?) i offset 152
    str     x17, [sp, #152]

    // Läs ESR_EL1 till x0 (trap-typ)
    mrs     x0, esr_el1

_vsyspatch:
    b       _vsyspatch    // patchas till vsys0()
   
    .text
    .global vtrap
    .type vtrap, %function

vtrap:
    // Allokera plats för trapframe
    sub     sp, sp, #TRAPFRAMESIZE

    // Spara registerpar till trapframe
    stp     x0,  x1,  [sp, #16]
    stp     x2,  x3,  [sp, #32]
    stp     x4,  x5,  [sp, #48]
    stp     x6,  x7,  [sp, #64]
    stp     x8,  x9,  [sp, #80]
    stp     x10, x11, [sp, #96]
    stp     x12, x13, [sp, #112]
    stp     x14, x15, [sp, #128]
    stp     x16, x17, [sp, #144]
    stp     x18, x19, [sp, #160]
    stp     x20, x21, [sp, #176]
    stp     x22, x23, [sp, #192]
    stp     x24, x25, [sp, #208]

    // Spara link register
    str     x30, [sp, #256]

    // Läs in ESR_EL1 (exception syndrome register) till x0
    mrs     x0, esr_el1

_vtrappatch:
    b       _vtrappatch    // patchas av runtime till riktig trap-handler


 .text
    .global virq
    .type virq, %function

virq:
    // Allokera plats för trapframe
    sub     sp, sp, #TRAPFRAMESIZE

    // Spara registerpar till trapframe
    stp     x0,  x1,  [sp, #16]
    stp     x2,  x3,  [sp, #32]
    stp     x4,  x5,  [sp, #48]
    stp     x6,  x7,  [sp, #64]
    stp     x8,  x9,  [sp, #80]
    stp     x10, x11, [sp, #96]
    stp     x12, x13, [sp, #112]
    stp     x14, x15, [sp, #128]
    stp     x16, x17, [sp, #144]
    stp     x18, x19, [sp, #160]
    stp     x20, x21, [sp, #176]
    stp     x22, x23, [sp, #192]
    stp     x24, x25, [sp, #208]

    // Spara link register
    str     x30, [sp, #256]

    // Lägg in typ (IRQ = 1 << 32)
    movz    x0, #0x0000, lsl #0
    movk    x0, #0x0000, lsl #16
    movk    x0, #0x0000, lsl #32
    movk    x0, #0x0002, lsl #48    // 1 << 32 = 0x0002000000000000

_virqpatch:
    b       _virqpatch    // patchas av runtime till vtrapX



    .text
    .global vfiq
    .type vfiq, %function

vfiq:
    // Allokera TRAPFRAMESIZE bytes på stacken
    sub     sp, sp, #TRAPFRAMESIZE

    // Spara registerpar till trapframe
    stp     x0,  x1,  [sp, #16]
    stp     x2,  x3,  [sp, #32]
    stp     x4,  x5,  [sp, #48]
    stp     x6,  x7,  [sp, #64]
    stp     x8,  x9,  [sp, #80]
    stp     x10, x11, [sp, #96]
    stp     x12, x13, [sp, #112]
    stp     x14, x15, [sp, #128]
    stp     x16, x17, [sp, #144]
    stp     x18, x19, [sp, #160]
    stp     x20, x21, [sp, #176]
    stp     x22, x23, [sp, #192]
    stp     x24, x25, [sp, #208]

    // Spara link register (x30)
    str     x30, [sp, #256]

    // Lägg in typ (FIQ = 2 << 32)
    movz    x0, #0x0000, lsl #0
    movk    x0, #0x0000, lsl #16
    movk    x0, #0x0000, lsl #32
    movk    x0, #0x0004, lsl #48    // 2 << 32 = 0x0004000000000000

_vfiqpatch:
    b       _vfiqpatch    // patchas vid runtime till vtrapX

    .text
    .global vserr
    .type vserr, %function

vserr:
    // Allokera plats på stacken
    sub     sp, sp, #TRAPFRAMESIZE

    // Spara registerpar till trapframe på stacken
    stp     x0,  x1,  [sp, #16]
    stp     x2,  x3,  [sp, #32]
    stp     x4,  x5,  [sp, #48]
    stp     x6,  x7,  [sp, #64]
    stp     x8,  x9,  [sp, #80]
    stp     x10, x11, [sp, #96]
    stp     x12, x13, [sp, #112]
    stp     x14, x15, [sp, #128]
    stp     x16, x17, [sp, #144]
    stp     x18, x19, [sp, #160]
    stp     x20, x21, [sp, #176]
    stp     x22, x23, [sp, #192]
    stp     x24, x25, [sp, #208]

    // Spara link register
    str     x30, [sp, #256]

    // Läs ESR_EL1 till x0
    mrs     x0, esr_el1

    // Skapa konstanten (3 << 32) = 0x0000000600000000
    movz    x1, #0x0000, lsl #0
    movk    x1, #0x0000, lsl #16
    movk    x1, #0x0000, lsl #32
    movk    x1, #0x0006, lsl #48    // 0x0006000000000000

    // Lägg till typen i ESR
    orr     x0, x0, x1

_vserrpatch:
    b       _vserrpatch    // patchas senare vid runtime


// peek.S – GCC/GAS-syntax för AArch64 (ARM64)
// void peek(const uint8_t *src, uint8_t *dst, uint32_t len);

    .text
    .global peek
    .type peek, %function

peek:
    // Argument enligt AAPCS64:
    // x0 = src
    // x1 = dst
    // w2 = len

    // Flytta in argument för att matcha Plan 9-logiken
    mov     x1, x9        // src -> x9 (motsvarar R1)
    mov     x0, x10       // dst -> x10 (motsvarar R2)
    mov     w2, w11       // len -> w11 (motsvarar R0)

    b       _peekinst

    .global _peekinst
    .type _peekinst, %function

_peekinst:
_peekloop:
    ldrb    w12, [x9], #1      // ladda byte från [x9], öka x9 med 1 (MOVBU (R1)1!)
    strb    w12, [x10], #1     // lagra byte till [x10], öka x10 med 1 (MOVBU R3, (R2)1!)
    subs    w11, w11, #1       // minska räknaren (SUBS $1, R0)
    b.ne    _peekloop          // om inte noll, fortsätt

    ret                        // RETURN


// Fil: hvccall.S

.global hvccall
.type hvccall, %function
hvccall:
    // Spara R26 och R27 (x26 och x27) på stacken
    stp     x26, x27, [sp, #-32]!

    // R0 = Ureg, flytta till x8
    mov     x8, x0

    // Spara ureg-pekaren på stacken
    str     x8, [sp, #16]

    // Ladda register från Ureg-strukturen
    ldp     x0, x1, [x8, #0]
    ldp     x2, x3, [x8, #16]
    ldp     x4, x5, [x8, #32]
    ldp     x6, x7, [x8, #48]

    // Gör hypervisor call
    hvc     #0

    // Återställ ureg-pekaren
    ldr     x8, [sp, #16]

    // Skriv tillbaka register till Ureg-strukturen
    stp     x0, x1, [x8, #0]
    stp     x2, x3, [x8, #16]

    // Återställ x26 och x27 från stacken
    ldp     x26, x27, [sp], #32

    ret