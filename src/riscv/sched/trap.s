        .section .text
        .option norvc
        .global TrapVector
        .align 4
TrapVector:
        csrr    t0, mcause
        li      t1, 0x8000000000000007
        beq     t0, t1, .Ltimer
        # Handle other traps here (syscalls, faults, etc.)
        # For now, just return
        mret
.Ltimer:
        jal NextProcess