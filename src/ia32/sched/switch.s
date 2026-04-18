        .section .text
        .global NextProcess
        .extern CurrentProc
        .extern ScratchProc
        .extern CommitProcessSave
        .extern CommitNextProcess
NextProcess:
        cli
        movl    %eax, (0*4+ScratchProc)
        movl    %ecx, (1*4+ScratchProc)
        movl    %edx, (2*4+ScratchProc)
        movl    %ebx, (3*4+ScratchProc)
        movl    %ebp, (5*4+ScratchProc)
        movl    %esi, (6*4+ScratchProc)
        movl    %edi, (7*4+ScratchProc)
        movl    $ScratchProc, %edi
        popl    8*4(%edi)
        popl    9*4(%edi)
        popl    15*4(%edi)
        movl    %esp, (4*4+ScratchProc)

        # --- SEGMENT REGISTERS ---
        movw    %ds, %ax
        movw    %es, %bx
        movw    %ss, %cx
        movw    %fs, %dx
        movw    %gs, %si
        movl    %eax, 10*4(%edi)
        movl    %ebx, 11*4(%edi)
        movl    %ecx, 12*4(%edi)
        movl    %edx, 13*4(%edi)
        movl    %esi, 14*4(%edi)

        movw    $0x10, %ax
        movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    %ax, %fs
	movw    %ax, %gs
        movl    $(InterruptStackTop-16), %esp
        xorl    %ebp,%ebp

        # --- SWITCH TO INTERRUPT STACK AND COMMIT ---
        call    CommitProcessSave
        call    CommitNextProcess

        # --- RELOAD POINTER (calls may have trashed edi) ---
        movl    $ScratchProc, %edi

        # --- RESTORE SEGMENT REGISTERS ---
        movl    10*4(%edi), %eax
        movl    11*4(%edi), %ebx
        movl    12*4(%edi), %ecx
        movl    13*4(%edi), %edx
        movl    14*4(%edi), %esi
        movw    %ax, %ds
        movw    %bx, %es
        movw    %cx, %ss
        movw    %dx, %fs
        movw    %si, %gs

        orl     $0x0202, 15*4(%edi)

        # --- RESTORE GPRS (edi last) ---
        movl    0*4(%edi), %eax
        movl    1*4(%edi), %ecx
        movl    2*4(%edi), %edx
        movl    3*4(%edi), %ebx
        movl    4*4(%edi), %esp
        movl    5*4(%edi), %ebp
        movl    6*4(%edi), %esi
        movl    7*4(%edi), %edi
        pushl   15*4+ScratchProc
        pushl   9*4+ScratchProc
        pushl   8*4+ScratchProc
        sti
        iret

        .section .bss
InterruptStack:
        .space  4096
InterruptStackTop: