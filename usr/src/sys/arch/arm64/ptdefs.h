#ifndef ARCH_ARM64_PTDEFS_H
#define ARCH_ARM64_PTDEFS_H

#define PT_ENTRIES      512
#define PT_SHIFT_L1     39
#define PT_SHIFT_L2     30
#define PT_SHIFT_L3     21
#define PT_PAGE_SHIFT   12

#define PT_MASK         0x1FFUL // 9 bit index

#define L1_INDEX(va)    (((va) >> PT_SHIFT_L1) & PT_MASK)
#define L2_INDEX(va)    (((va) >> PT_SHIFT_L2) & PT_MASK)
#define L3_INDEX(va)    (((va) >> PT_SHIFT_L3) & PT_MASK)

#define PAGE_ALIGN_DOWN(x) ((x) & ~((1UL << PT_PAGE_SHIFT) - 1))

// Page table entry flags
#define PTE_VALID       (1UL << 0)
#define PTE_TABLE       (1UL << 1)
#define PTE_AF          (1UL << 10)
#define PTE_USER        (1UL << 6)
#define PTE_RW          (0UL << 7)
#define PTE_RO          (1UL << 7)
#define PTE_SH_INNER    (3UL << 8)
#define PTE_ATTR_IDX(n) ((n) << 2)
#define PTE_UXN         (1UL << 54)
#define PTE_PXN         (1UL << 53)

#define ATTR_IDX_NORMAL 0
#define ATTR_IDX_DEVICE 1

#endif