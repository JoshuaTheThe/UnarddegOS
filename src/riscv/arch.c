
#include <arch.h>
#include <stdint.h>

extern void TrapVector(void);

void InitTraps(void)
{
        uintptr_t tvec = (uintptr_t)TrapVector;
        __asm volatile("csrw mtvec, %0" :: "r"(tvec));
}

void ArchInitialise(unsigned int a, unsigned int b)
{
        (void)a;
        (void)b;
        InitTraps();
}

void LoadModules(unsigned int a, unsigned int b)
{(void)a; (void)b;}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: RISCV, CPU: Generic\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile("csrc mie, %0" :: "r"(1 << 7));
        __asm volatile ("csrrci zero, mstatus, 0x8");
}

void ArchSti(void)
{
        __asm volatile("csrs mie, %0" :: "r"(1 << 7));
        __asm volatile ("csrrsi zero, mstatus, 0x8");
}
