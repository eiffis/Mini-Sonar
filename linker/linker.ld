ENTRY(Reset_Handler)

MEMORY
{
    FLASH(rx) :
        ORIGIN = 0x08000000,
        LENGTH = 2048K
    SRAM1(rwx) :
        ORIGIN = 0x20000000,
        LENGTH = 192K
}


SECTIONS
{
    .isr_vector ORIGIN(FLASH):
    {
        KEEP(*(.isr_vector))
        KEEP(*(.isr_vector*))
    } > FLASH

    .text :
    {
        *(.text)
        *(.text*)
        . = ALIGN(4);
    } > FLASH

    .rodata :
    {
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data : 
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
    } > SRAM1 AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
    } > SRAM1

    _sidata = LOADADDR(.data);
    _sdata = ADDR(.data);
    _edata = ADDR(.data) + SIZEOF(.data);

    _sbss = ADDR(.bss);
    _ebss = ADDR(.bss) + SIZEOF(.bss);

    _estack = ORIGIN(SRAM1) + LENGTH(SRAM1);
}