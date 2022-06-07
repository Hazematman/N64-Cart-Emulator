SECTIONS
{
    . = 0x45000000;
    .text : {
        PROVIDE(__text_start = .);
        *(.text.start)
        *(.text*)
        . = ALIGN(4096);
        PROVIDE(__text_end = .);
    }
    .data : {
        PROVIDE(__data_start = .);
        . = ALIGN(16);
        *(.rodata*);
        *(.data .data.*)
        PROVIDE(__data_end = .);
    }
    . += 1024;
    PROVIDE(__stack_start = .);
    . = ALIGN(16);
    . += 4096;
    PROVIDE(__stack_end = .);

    /DISCARD/ :
    {
        *(.riscv.attributes);
        *(.comment);
    }
}
