
#include <arch.h>
#include <sched/trap.h>

extern void TimerVector(void);
extern void DefaultVector(void);

void InitGIC(void)
{
        GICD_CTLR = 1;
        GICD_ISENABLER1 |= (1 << (34 - 32));  // IRQ 34 = bit 2 of ISENABLER1
        GICC_PMR  = 0xFF;
        GICC_CTLR = 1;
}

void InitTimer(void)
{
        InitGIC();
        InitGIC();
        TIMER_LOAD = 1000000;
        TIMER_CTRL = (1 << 7)
                   | (1 << 6)
                   | (1 << 5)
                   | (1 << 1);
}

void InitTraps(void)
{
        extern void TrapVector(void);
        __asm volatile("mcr p15, 0, %0, c12, c0, 0" :: "r"(TrapVector));
        __asm volatile(
            "mrs r0, cpsr\n"
            "bic r0, r0, #0x80\n"
            "msr cpsr_c, r0\n"
            ::: "r0"
        );
}

void ArchInitialise(unsigned int a, unsigned int b)
{
        (void)a;
        (void)b;
        InitTraps();
        InitTimer();
}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: Arm32,Arm/v7, CPU: cortex-a15\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile ("cpsid i");
}

void ArchSti(void)
{
        __asm volatile ("cpsie i");
}
