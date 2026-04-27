ENTRY(_start)

SECTIONS
{
	. = 2M;

        .text : {
                *(.multiboot)
                *(.text)
                *(.text.*)      /* Catch all .text.* sections */
                *(.text*)
        }
        
        .rodata : {
                *(.rodata)
                *(.rodata.*)    /* Catch all .rodata.* sections */
                *(.rodata*)     /* Catch any other rodata variants */
        }
        
        .data : {
                *(.data)
                *(.data.*)      /* Catch all .data.* sections */
                *(.got)         /* Global offset table */
                *(.got.*)       /* GOT entries */
                *(.got.plt)     /* GOT PLT entries */
                *(.plt)         /* Procedure linkage table */
                *(.plt.*)
                *(.iplt)        /* Inline PLT */
                *(.igot.*)
                *(.toc)         /* Table of contents (PPC) */
                *(.toc.*)
        }
        
        .bss : {
                *(COMMON)
                *(.bss)
                *(.bss.*)       /* Catch all .bss.* sections */
                *(COMMON.*)
        }

        . = ALIGN(4);
        .init : {
                *(.init)
                *(.init.*)
        }
        
        .fini : {
                *(.fini)
                *(.fini.*)
        }
        
        .ctors : {
                *(.ctors)
                *(.ctors.*)
                *(.init_array)
                *(.init_array.*)
        }
        
        .dtors : {
                *(.dtors)
                *(.dtors.*)
                *(.fini_array)
                *(.fini_array.*)
        }
        
        .jcr : {
                *(.jcr)
        }
        
        .debug : {
                *(.debug)
                *(.debug.*)
        }
        
        .stab : {
                *(.stab)
                *(.stab.*)
        }
        
        .stabstr : {
                *(.stabstr)
                *(.stabstr.*)
        }
        
        . = ALIGN(4);

        /DISCARD/ : {
                *(.note.GNU-stack)
                *(.gnu_debuglink)
                *(.gnu.lto_*)
                *(.comment)
                *(.comment.*)
        }
}
