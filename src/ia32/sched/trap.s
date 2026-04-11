        .section .text
        .global IdtTimer
        .global IdtDefault
        .extern NextProcess
IdtTimer:
        cli
        jmp NextProcess
IdtDefault:
        iret