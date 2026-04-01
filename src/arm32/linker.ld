ENTRY(_start)
SECTIONS {
        . = 0x80010000;   /* vexpress-a15 RAM base */
        .text : {
                *(.stub)
                *(.text)
        }
        .data : { *(.data) }
        .bss  : {
                _bss_start = .;
                *(.bss)
                _bss_end = .;
        }
        _stack_base = .;
        . += 0x10000;  /* Reserve 64KB for the stack */
        _stack_top = .;
}