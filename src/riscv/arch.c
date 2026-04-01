
#include <arch.h>

// Stub
void ArchInitialise(void) {}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: RISCV, CPU: Generic\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile ("csrrci zero, mstatus, 0x8");
}

void ArchSti(void)
{
        __asm volatile ("csrrsi zero, mstatus, 0x8");
}
