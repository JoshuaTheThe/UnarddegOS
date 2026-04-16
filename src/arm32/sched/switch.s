.section .text
        .global NextProcess
        .extern ScratchProc
        .extern CommitProcessSave
        .extern CommitNextProcess
NextProcess:
        sub     lr, lr, #4                  @ correct return address
        @ Save lr and spsr BEFORE switching mode
        mov     r12, lr                     @ stash lr_irq
        mrs     r11, spsr                   @ stash spsr_irq
        cps     #0x13                       @ switch to SVC mode
        @ Now in SVC, r11=cpsr r12=pc, safe to use ScratchProc
        ldr     r10, =ScratchProc
        stmia   r10, {r0-r9}                @ slots 0-9 (r10-r12 saved manually)
        str     r10, [r10, #10*4]           @ slot 10
        str     r11, [r10, #11*4]           @ slot 11
        str     r12, [r10, #12*4]           @ slot 12
        str     sp,  [r10, #13*4]           @ slot 13 = sp_svc
        str     lr,  [r10, #14*4]           @ slot 14 = lr_svc
        str     r12, [r10, #15*4]           @ slot 15 = PC (was lr_irq-4)
        str     r11, [r10, #16*4]           @ slot 16 = cpsr (was spsr_irq)
        ldr     sp,  =InterruptStack + 4096
        bl      CommitProcessSave
        bl      CommitNextProcess
        ldr     r10, =ScratchProc
        ldr     r0,  [r10, #16*4]
        msr     spsr_cxsf, r0
        ldr     lr,  [r10, #15*4]
        ldr     sp,  [r10, #13*4]
        ldr     r14, [r10, #14*4]
        ldmia   r10, {r0-r12}
        movs    pc, lr
        .section .bss
InterruptStack:
        .space  4096