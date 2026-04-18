ENTRY(Init)
SECTIONS
{
        . = 0;
        .ModuleHeader : { *(.ModuleHeader) }
        .text : { *(.text) }
        .data : { *(.data) }
        .rodata : { *(.rodata) }
        .bss : { *(.bss) }
}
