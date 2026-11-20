ENTRY(Reset_Handler)

MEMORY
{
    flash(rx) :
        ORIGIN = 0x08000000,
        LENGTH = 2048K
    sram1(rwx) :
        ORIGIN = 0x20000000,
        LENGTH = 192K
}


SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
        KEEP(*(.isr_vector*))
        . = ALIGN(4);
    } > flash

    .text :
    {
        *(.text)
        *(.text*)
        . = ALIGN(4);
    } > flash

    .rodata :
    {
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > flash

    .data : 
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
    } > sram1 AT> flash

    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
    } > sram1

    _sidata = LOADADDR(.data);
    _sdata = ADDR(.data);
    _edata = ADDR(.data) + SIZEOF(.data);

    _sbss = ADDR(.bss);
    _ebss = ADDR(.bss) + SIZEOF(.bss);

    _estack = ORIGIN(sram1) + LENGTH(sram1);
}