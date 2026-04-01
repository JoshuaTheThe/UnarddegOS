        .global _start
        .extern kmain
        .section .stub
_start: cpsid if
        ldr r0, =_bss_start
        ldr r1, =_bss_end
        mov r2, #0
.Lbss_loop:
        cmp r0, r1
        strlt r2, [r0], #4
        blt .Lbss_loop
        ldr sp, =_stack_top
        bl kmain
.Lhalt: b .Lhalt
        .size _start, . - _start