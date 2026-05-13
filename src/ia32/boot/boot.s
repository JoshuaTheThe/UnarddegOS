        .section .multiboot
        .align 8
.section .multiboot
.align 8
multiboot_header:
    .long 0xE85250D6
    .long 0
    .long (multiboot_header_end - multiboot_header)
    .long -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))
    
    /* Information request tag (type 1) */
    .align 8
    .word 1                    /* type */
    .word 0                    /* flags */
    .long 8 + 12               /* size: 8 header + 3*4 */
    .long 0                    /* request basic info (tag type 0) */
    .long 6                    /* request memory map (tag type 6) */
    .long 8                    /* request framebuffer info (tag type 8) */
    .long 0                    /* terminator */
    
    /* Console tag (type 4), optional */
    .align 8
    .word 4                    /* type */
    .word 0                    /* flags */
    .long 8                    /* size */
    
    /* Framebuffer tag (type 5) */
    .align 8
    .word 5                    /* type */
    .word 1                    /* flags (0=linear, 1=preferred) */
    .long 20                   /* size: 20 bytes total */
    .long 1024                 /* width */
    .long 768                  /* height */
    .long 32                   /* depth */
    
    /* End tag (type 0) */
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
