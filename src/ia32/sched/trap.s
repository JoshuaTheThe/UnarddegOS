        .section .text
        .global IdtTimer
        .global IdtDefault
        .extern NextProcess
IdtTimer:
IdtDefault:
        cli
        jmp NextProcess