
#include <arch.h>

// Stub
void ArchInitialise(void) {}

const char *ArchIdentify(void)
{
        static const char *Architecture = " [Info] ISA: RISCV, CPU: Generic";
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
