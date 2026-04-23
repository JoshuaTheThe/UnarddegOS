        .section .multiboot
        .align 8
multiboot_header:
        .long 0xE85250D6
        .long 0
        .long (multiboot_header_end - multiboot_header)
        .long -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))  /* Fixed checksum */
        
        .align 8
        .word 1          /* Tag type: required end */
        .word 0          /* Flags */
        .long 8          /* Tag size */
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
        .equ PRESENT, 128
        .equ NOT_SYS, 16
        .equ EXEC, 8
        .equ DC, 4
        .equ RW, 2
        .equ ACCESSED, 1
        .equ GRAN_4K, 128
        .equ SZ_32, 64
        .equ LONG_MODE, 32


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
        xorl %edx, %edx           /* Clear upper 32 bits (no NX) */
1:      movl %ebx, (%edi)
        movl %edx, 4(%edi)        /* Upper 32 bits = 0 -> execute allowed */
        addl $PAGE_SIZE, %ebx
        addl $SIZEOF_PT_ENTRY, %edi
        loop 1b

        /* PT2: identity mapping */
        movl $PT2_ADDR, %edi
        movl $(PT_PRESENT | PT_WRITABLE), %ebx
        movl $ENTRIES_PER_PT, %ecx
        xorl %edx, %edx
2:      movl %ebx, (%edi)
        movl %edx, 4(%edi)        /* Upper 32 bits = 0 */
        addl $PAGE_SIZE, %ebx
        addl $SIZEOF_PT_ENTRY, %edi
        loop 2b

PAEEnable:
        movl %cr4, %eax
        orl  $CR4_PAE_ENABLE, %eax
        movl %eax, %cr4

Compatibility:
        movl $EFER_MSR, %ecx
        rdmsr
        orl  $EFER_LM_ENABLE, %eax
        wrmsr
        movl $PML4T_ADDR, %eax
        movl %eax, %cr3

        movl %cr0, %eax
        orl  $CR0_PG_ENABLE, %eax
        movl %eax, %cr0

        lgdt (GDT.Pointer)
        ljmp $8,$Realm64
Realm64:
        .code64
_start64:
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        movw %ax, %ss
        movq $stack_top, %rsp
        andq $~0xF, %rsp
        subq $16, %rsp
        xorq %rbp, %rbp
        movq args, %rdi
        movq args+8, %rsi
        callq kmain

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
GDT:
        .quad 0
        .Code.limit_lo: .word 0xffff
        .Code.base_lo: .word 0
        .Code.base_mid: .byte 0
        .Code.access: .byte (PRESENT | NOT_SYS | EXEC | RW)
        .Code.flags: .byte GRAN_4K | LONG_MODE | 0xF   # Flags & Limit (high, bits 16-19)
        .Code.base_hi: .byte 0
        .Data.limit_lo: .word 0xffff
        .Data.base_lo: .word 0
        .Data.base_mid: .byte 0
        .Data.access: .byte PRESENT | NOT_SYS | RW
        .Data.Flags: .byte GRAN_4K | SZ_32 | 0xF       # Flags & Limit (high, bits 16-19)
        .Data.base_hi: .byte 0
GDT.Pointer:
        .word GDT.Pointer - GDT - 1
        .quad GDT
        .section .bss
        .align 16
stack_bottom:
        .skip 65536
stack_top:
args:
        .quad 0
        .quad 0
