.section .text
        .global TrapVector
TrapVector:
        b       .                   @ Reset        - hang
        b       .                   @ Undefined    - hang
        b       SvcHandler          @ SVC          - syscalls later
        b       .                   @ Prefetch Abort - hang
        b       .                   @ Data Abort   - hang
        nop                         @ Reserved
        b       IrqHandler          @ IRQ          - timer → NextProcess
        b       .                   @ FIQ          - hang
IrqHandler:
        @ Equivalent of: csrr t0, mcause / bltz t0, NextProcess
        @ On ARMv7 we already know we're in IRQ mode = it's an interrupt
        @ so unconditionally jump to NextProcess (no mcause check needed)
        b       NextProcess
SvcHandler:
        @ Equivalent of the non-interrupt trap path
        @ For now, just hang
        b       .
        movs    pc, lr              @ (unreachable, mirrors your mret)