        .section .multiboot
        .align 8
multiboot_header:
        /* Header magic */
        .long 0xE85250D6
        /* Architecture: i386 (0) */
        .long 0
        /* Header length */
        .long (multiboot_header_end - multiboot_header)
        /* Checksum */
        .long 0x100000000 - (0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))
        
        /* REQUIRED: Information request tag (tells GRUB what to give you) */
        .align 8
        .word 1
        .word 0
        .long 8
        .long 0    /* Request basic info */
        
        /* REQUIRED: End tag */
        .align 8
        .word 0
        .word 0
        .long 8
multiboot_header_end:
        .section .text
        .global _start
        .type _start, @function
_start: mov $stack_top, %esp
        xor %ebp, %ebp
        push %ebx
        push %eax
        call kmain
1:      cli
        hlt
        jmp 1b
        .size _start, . - _start
        .section .bss
        .align 16
stack_bottom:
        .skip 65536*4
stack_top: