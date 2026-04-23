.section .multiboot
        .align 8
multiboot_header:
        .long 0xE85250D6
        .long 0
        .long (multiboot_header_end - multiboot_header)
        .long 0x100000000 - (0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))
        .align 8
        .word 1
        .word 0
        .long 8
        .long 0
        .align 8
        .word 0
        .word 0
        .long 8
multiboot_header_end:

        .equ CR0_PAGING,        0x80000000
        .equ PML4T_ADDR,        0x1000
        .equ PDPT_ADDR,         0x2000
        .equ PDT_ADDR,          0x3000
        .equ PT_ADDR,           0x4000      /* virt 0x200000 -> phys 0x0 (kernel) */
        .equ PT2_ADDR,          0x5000      /* virt 0x0      -> phys 0x0 (identity, stack) */
        .equ SIZEOF_PAGE_TABLE, 4096
        .equ PT_PRESENT,        1
        .equ PT_WRITABLE,       2
        .equ ENTRIES_PER_PT,    512
        .equ SIZEOF_PT_ENTRY,   8
        .equ PAGE_SIZE,         0x1000
        .equ CR4_PAE_ENABLE,    0x20
        .equ EFER_MSR,          0xC0000080
        .equ EFER_LM_ENABLE,    0x100
        .equ CR0_PM_ENABLE,     0x1
        .equ CR0_PG_ENABLE,     CR0_PAGING

        .section .text
        .code32
        .global _start
        .type _start, @function
_start:
        cli
        cmpl $0x36D76289, %eax
        jne .hang32
        movl %eax, args
        movl %ebx, args+8

disable32bitPaging:
        movl %cr0, %eax
        andl $~CR0_PAGING, %eax
        movl %eax, %cr0

ClearPages:
        /* Zero 5 page tables: PML4T, PDPT, PDT, PT, PT2 (0x1000-0x5FFF) */
        movl $PML4T_ADDR, %edi
        movl %edi, %cr3
        xorl %eax, %eax
        movl $(5 * SIZEOF_PAGE_TABLE / 4), %ecx
        rep stosl

PageLink:
        /* PML4[0] -> PDPT */
        movl $PML4T_ADDR, %edi
        movl $(PDPT_ADDR | PT_PRESENT | PT_WRITABLE), (%edi)

        /* PDPT[0] -> PDT */
        movl $PDPT_ADDR, %edi
        movl $(PDT_ADDR | PT_PRESENT | PT_WRITABLE), (%edi)

        /* PDT[0] -> PT2  (virt 0x000000 - 0x1FFFFF, identity mapped) */
        movl $PDT_ADDR, %edi
        movl $(PT2_ADDR | PT_PRESENT | PT_WRITABLE), (%edi)

        /* PDT[1] -> PT   (virt 0x200000 - 0x3FFFFF -> phys 0x200000 - 0x3fffff) */
        movl $(PT_ADDR | PT_PRESENT | PT_WRITABLE), 8(%edi)

PageFill:
        movl $PT_ADDR, %edi
        movl $(0x200000 | PT_PRESENT | PT_WRITABLE), %ebx
        movl $ENTRIES_PER_PT, %ecx
1:      movl %ebx, (%edi)
        addl $PAGE_SIZE, %ebx
        addl $SIZEOF_PT_ENTRY, %edi
        loop 1b

        /* PT2: virt 0x0+N -> phys 0x0+N (identity map, covers stack) */
        movl $PT2_ADDR, %edi
        movl $(PT_PRESENT | PT_WRITABLE), %ebx
        movl $(8 * SIZEOF_PAGE_TABLE), %ecx
2:      movl %ebx, (%edi)
        addl $PAGE_SIZE, %ebx
        addl $SIZEOF_PT_ENTRY, %edi
        loop 2b

        lgdt (gdt64_ptr)
PAEEnable:
        movl %cr4, %eax
        orl  $CR4_PAE_ENABLE, %eax
        movl %eax, %cr4

Compatibility:
        movl $EFER_MSR, %ecx
        rdmsr
        orl  $EFER_LM_ENABLE, %eax
        wrmsr

        pushl    $8
        pushl    $(_start64 + 0x200000)
        lret

        .code64
_start64:
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        movw %ax, %ss
        movq $stack_top, %rsp
        xorq %rbp, %rbp
        movl args, %edi
        movl args+8, %esi
        call kmain

.hang64:
        cli
        hlt
        jmp .hang64

        .code32
.hang32:
        cli
        hlt
        jmp .hang32

        .align 8
        .size _start, . - _start
        .section .data
gdt64:
        .quad 0x0000000000000000
        .quad 0x00af9a000000ffff
        .quad 0x00cf92000000ffff
gdt64_end:

gdt64_ptr:
        .word gdt64_end - gdt64 - 1
        .quad gdt64
        .section .bss
        .align 16
stack_bottom:
        .skip 65536
stack_top:
args:
        .quad 0
        .quad 0
