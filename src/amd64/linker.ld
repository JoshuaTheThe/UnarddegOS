ENTRY(_start)

SECTIONS
{
	. = 2M;
        . = ALIGN(4K);
        
        .text : {
                *(.multiboot)
                *(.text)
                *(.text.*)
        }
        
        .rodata : {
                *(.rodata)
                *(.rodata.*)
        }
        
        .data : {
                *(.data)
                *(.data.*)
        }
        
        .bss : {
                *(COMMON)
                *(.bss)
                *(.bss.*)
                *(COMMON.*)
        }

        . = ALIGN(4K);

        /DISCARD/ : {
                *(.note.GNU-stack)
                *(.gnu_debuglink)
                *(.gnu.lto_*)
                *(.comment)
                *(.comment.*)
        }
}
