        .section .text
        .global NextProcess
        .extern CurrentProc
        .extern ScratchProc
        .extern CommitProcessSave
        .extern CommitNextProcess
NextProcess:
        cli
        movq    %rax, (0*8+ScratchProc)
        movq    %rcx, (1*8+ScratchProc)
        movq    %rdx, (2*8+ScratchProc)
        movq    %rbx, (3*8+ScratchProc)
        movq    %rbp, (5*8+ScratchProc)
        movq    %rsi, (6*8+ScratchProc)
        movq    %rdi, (7*8+ScratchProc)
        movq    %r8,  (16*8+ScratchProc)
        movq    %r9,  (17*8+ScratchProc)
        movq    %r10, (18*8+ScratchProc)
        movq    %r11, (19*8+ScratchProc)
        movq    %r12, (20*8+ScratchProc)
        movq    %r13, (21*8+ScratchProc)
        movq    %r14, (22*8+ScratchProc)
        movq    %r15, (23*8+ScratchProc)
        movq    $ScratchProc, %rdi
        popq    8*8(%rdi)
        popq    9*8(%rdi)
        popq    15*8(%edi)
        movq    %rsp, (4*8+ScratchProc)

        # --- SEGMENT REGISTERS ---
        movw    %ds, %ax
        movw    %es, %bx
        movw    %ss, %cx
        movw    %fs, %dx
        movw    %gs, %si
        movq    %rax, 10*8(%rdi)
        movq    %rbx, 11*8(%rdi)
        movq    %rcx, 12*8(%rdi)
        movq    %rdx, 13*8(%rdi)
        movq    %rsi, 14*8(%rdi)

        movw    $0x10, %ax
        movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    %ax, %fs
	movw    %ax, %gs
        movq    $(InterruptStackTop-16), %rsp
        xorq    %rbp,%rbp

        # --- SWITCH TO INTERRUPT STACK AND COMMIT ---
        call    CommitProcessSave
        call    CommitNextProcess

        # --- RELOAD POINTER (calls may have trashed edi) ---
        movq    $ScratchProc, %rdi

        # --- RESTORE SEGMENT REGISTERS ---
        movq    10*8(%rdi), %rax
        movq    11*8(%rdi), %rbx
        movq    12*8(%rdi), %rcx
        movq    13*8(%rdi), %rdx
        movq    14*8(%rdi), %rsi
        movw    %ax, %ds
        movw    %bx, %es
        movw    %cx, %ss
        movw    %dx, %fs
        movw    %si, %gs

        orq     $0x0202, 15*8(%rdi)

        # --- RESTORE GPRS (edi last) ---
        movq    0*8(%rdi), %rax
        movq    1*8(%rdi), %rcx
        movq    2*8(%rdi), %rdx
        movq    3*8(%rdi), %rbx
        movq    4*8(%rdi), %rsp
        movq    5*8(%rdi), %rbp
        movq    6*8(%rdi), %rsi
        movq    16*8(%rdi), %r8
        movq    17*8(%rdi), %r9
        movq    18*8(%rdi), %r10
        movq    19*8(%rdi), %r11
        movq    20*8(%rdi), %r12
        movq    21*8(%rdi), %r13
        movq    22*8(%rdi), %r14
        movq    23*8(%rdi), %r15
        movq    7*8(%rdi), %rdi
        pushq   15*8+ScratchProc
        pushq   9*8+ScratchProc
        pushq   8*8+ScratchProc
        sti
        iretq

        .section .bss
        .align 16
InterruptStack:
        .space  4096
InterruptStackTop: