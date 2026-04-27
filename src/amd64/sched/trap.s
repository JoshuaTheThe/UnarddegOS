        .section .text
        .global IdtTimer
        .global IdtDefault
        .extern NextProcess
        .extern PanicImpl
IdtTimer:
        cli
        jmp NextProcess
IdtDefault:
        cli
        movq $File, %rdi
        movq $9,    %rsi
        movq $6,    %rdx
        movq $Code, %rcx
        call PanicImpl
1:      pause
        hlt
        jmp 1b
        .section .data
File:   .asciz "src/amd64/sched/trap.s\0"
Code:   .asciz "PANIC_UNHANDLED_INTERRUPT\0"