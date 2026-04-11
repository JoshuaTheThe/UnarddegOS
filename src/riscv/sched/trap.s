        .section .text
        .option norvc
        .global TrapVector
        .align 4
TrapVector:
        csrrci zero, mstatus, 0x8
        csrw   mscratch, t0
        csrr   t0, mcause
        bltz   t0, NextProcess
        csrr   t0, mscratch
        # csrrsi zero, mstatus, 0x8
        # Handle other traps here (syscalls, faults, etc.)
        # For now, just hang
hang:
        j hang
        mret