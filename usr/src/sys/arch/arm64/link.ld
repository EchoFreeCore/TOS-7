/* link.ld – för AArch64 kernel.elf för QEMU -kernel */

ENTRY(_start)

SECTIONS
{
    /* Ladda kärnan på 0x80000, QEMU förväntar sig det */
    . = 0x80000;

    /* === TEXT-sektionen: kod === */
    .text : ALIGN(8) {
        _text_start = .;
        KEEP(*(.text.boot))      /* _start och init */
        *(.text .text.*)
        _text_end = .;
    }

    /* === RODATA-sektionen: konstantdata === */
    .rodata : ALIGN(8) {
        _rodata_start = .;
        *(.rodata .rodata.*)
        _rodata_end = .;
    }

    /* === DATA-sektionen: initierad data === */
    .data : ALIGN(8) {
        _data_start = .;
        *(.data .data.*)
        _data_end = .;
    }

    /* === BSS-sektionen: noll-initierad data === */
    .bss : ALIGN(8) {
        _bss_start = .;
        *(.bss .bss.* COMMON)
        _bss_end = .;
    }

    /* === Heap/stack marker === */
    _end = .;

    /* För symboler i C */
    __bss_start = _bss_start;
    __bss_end   = _bss_end;
}
