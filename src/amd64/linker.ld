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
                *(.got)         /* GOT works differently in x86_64 */
                *(.got.*)
                *(.got.plt)
                *(.plt)         /* PLT for 64-bit */
                *(.plt.*)
                *(.iplt)
                *(.igot.*)
        }
        
        .bss : {
                *(COMMON)
                *(.bss)
                *(.bss.*)
                *(COMMON.*)
        }

        . = ALIGN(4K);
        
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
        
        . = ALIGN(4K);

        /DISCARD/ : {
                *(.note.GNU-stack)
                *(.gnu_debuglink)
                *(.gnu.lto_*)
                *(.comment)
                *(.comment.*)
        }
}
