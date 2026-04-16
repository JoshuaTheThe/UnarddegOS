        .section .text
        .option norvc
        .type ContextSwitch, @function
        .global NextProcess
        .extern CurrentProc
        .extern ScratchProc
        .extern CommitProcessSave
        .extern CommitNextProcess
        .extern InterruptStack
NextProcess:
        # GPR SAVE
        la      t0, ScratchProc
        sd      x1,  1*8(t0)
        sd      x2,  2*8(t0)
        sd      x3,  3*8(t0)
        sd      x4,  4*8(t0)
        sd      x6,  6*8(t0)
        sd      x7,  7*8(t0)
        sd      x8,  8*8(t0)
        sd      x9,  9*8(t0)
        sd      x10, 10*8(t0)
        sd      x11, 11*8(t0)
        sd      x12, 12*8(t0)
        sd      x13, 13*8(t0)
        sd      x14, 14*8(t0)
        sd      x15, 15*8(t0)
        sd      x16, 16*8(t0)
        sd      x17, 17*8(t0)
        sd      x18, 18*8(t0)
        sd      x19, 19*8(t0)
        sd      x20, 20*8(t0)
        sd      x21, 21*8(t0)
        sd      x22, 22*8(t0)
        sd      x23, 23*8(t0)
        sd      x24, 24*8(t0)
        sd      x25, 25*8(t0)
        sd      x26, 26*8(t0)
        sd      x27, 27*8(t0)
        sd      x28, 28*8(t0)
        sd      x29, 29*8(t0)
        sd      x30, 30*8(t0)
        sd      x31, 31*8(t0)
        csrr    t1, mscratch
        sd      t1,  5*8(t0)        # save original t0 (x5)

        # PC + MSTATUS SAVE
        csrr    t1, mepc
        sd      t1,  0*8(t0)        # slot 0 = PC
        csrr    t1, mstatus
        sd      t1, 64*8(t0)        # slot 64 = mstatus
        csrr    t1, satp
        sd      t1, 66*8(t0)

        # # FPU SAVE - enable FS bits so fsd doesn't trap
        li      t1, 0x6000
        csrs    mstatus, t1
        fsd     f0,  32*8(t0)
        fsd     f1,  33*8(t0)
        fsd     f2,  34*8(t0)
        fsd     f3,  35*8(t0)
        fsd     f4,  36*8(t0)
        fsd     f5,  37*8(t0)
        fsd     f6,  38*8(t0)
        fsd     f7,  39*8(t0)
        fsd     f8,  40*8(t0)
        fsd     f9,  41*8(t0)
        fsd     f10, 42*8(t0)
        fsd     f11, 43*8(t0)
        fsd     f12, 44*8(t0)
        fsd     f13, 45*8(t0)
        fsd     f14, 46*8(t0)
        fsd     f15, 47*8(t0)
        fsd     f16, 48*8(t0)
        fsd     f17, 49*8(t0)
        fsd     f18, 50*8(t0)
        fsd     f19, 51*8(t0)
        fsd     f20, 52*8(t0)
        fsd     f21, 53*8(t0)
        fsd     f22, 54*8(t0)
        fsd     f23, 55*8(t0)
        fsd     f24, 56*8(t0)
        fsd     f25, 57*8(t0)
        fsd     f26, 58*8(t0)
        fsd     f27, 59*8(t0)
        fsd     f28, 60*8(t0)
        fsd     f29, 61*8(t0)
        fsd     f30, 62*8(t0)
        fsd     f31, 63*8(t0)
        frcsr   t1
        sd      t1, 65*8(t0)        # slot 65 = fcsr
        la      sp, InterruptStack+4096
        # STAGE TWO - commit scratch -> CurrentProc
        call    CommitProcessSave 
        # STAGE THREE - advance CurrentProc = CurrentProc->next
        call    CommitNextProcess
        # STAGE FOUR - LOAD next task from ScratchProc
        la      t0, ScratchProc
        ld      t1, 66*8(t0)
        csrw    satp, t1
        sfence.vma
        # Restore PC and mstatus into CSRs first
        ld      t1,  0*8(t0)
        csrw    mepc, t1
        # Restore fcsr
        ld      t1, 65*8(t0)
        fscsr   t1
        # FPU RESTORE
        fld     f0,  32*8(t0)
        fld     f1,  33*8(t0)
        fld     f2,  34*8(t0)
        fld     f3,  35*8(t0)
        fld     f4,  36*8(t0)
        fld     f5,  37*8(t0)
        fld     f6,  38*8(t0)
        fld     f7,  39*8(t0)
        fld     f8,  40*8(t0)
        fld     f9,  41*8(t0)
        fld     f10, 42*8(t0)
        fld     f11, 43*8(t0)
        fld     f12, 44*8(t0)
        fld     f13, 45*8(t0)
        fld     f14, 46*8(t0)
        fld     f15, 47*8(t0)
        fld     f16, 48*8(t0)
        fld     f17, 49*8(t0)
        fld     f18, 50*8(t0)
        fld     f19, 51*8(t0)
        fld     f20, 52*8(t0)
        fld     f21, 53*8(t0)
        fld     f22, 54*8(t0)
        fld     f23, 55*8(t0)
        fld     f24, 56*8(t0)
        fld     f25, 57*8(t0)
        fld     f26, 58*8(t0)
        fld     f27, 59*8(t0)
        fld     f28, 60*8(t0)
        fld     f29, 61*8(t0)
        fld     f30, 62*8(t0)
        fld     f31, 63*8(t0)
        # GPR RESTORE - t0 (x5) absolutely last, it's our pointer
        ld      x1,  1*8(t0)
        ld      x2,  2*8(t0)
        ld      x3,  3*8(t0)
        ld      x4,  4*8(t0)
        ld      x6,  6*8(t0)
        ld      x7,  7*8(t0)
        ld      x8,  8*8(t0)
        ld      x9,  9*8(t0)
        ld      x10, 10*8(t0)
        ld      x11, 11*8(t0)
        ld      x12, 12*8(t0)
        ld      x13, 13*8(t0)
        ld      x14, 14*8(t0)
        ld      x15, 15*8(t0)
        ld      x16, 16*8(t0)
        ld      x17, 17*8(t0)
        ld      x18, 18*8(t0)
        ld      x19, 19*8(t0)
        ld      x20, 20*8(t0)
        ld      x21, 21*8(t0)
        ld      x22, 22*8(t0)
        ld      x23, 23*8(t0)
        ld      x24, 24*8(t0)
        ld      x25, 25*8(t0)
        ld      x26, 26*8(t0)
        ld      x27, 27*8(t0)
        ld      x28, 28*8(t0)
        ld      x29, 29*8(t0)
        ld      x30, 30*8(t0)
        ld      x31, 31*8(t0)
        ld      t1, 64*8(t0)
        csrw    mstatus, t1
        ld      t0,  5*8(t0)
        mret                        # jumps to mepc, privilege from mstatus
InterruptStack:
        .space 4096