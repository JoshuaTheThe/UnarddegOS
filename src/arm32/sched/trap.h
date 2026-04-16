
#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

#define GIC_DIST_BASE   0x2C001000
#define GIC_CPU_BASE    0x2C002000
#define GICD_CTLR       (*(volatile uint32_t*)(GIC_DIST_BASE + 0x000))
#define GICD_ISENABLER1 (*(volatile uint32_t*)(GIC_DIST_BASE + 0x104))
#define GICC_CTLR       (*(volatile uint32_t*)(GIC_CPU_BASE  + 0x000))
#define GICC_PMR        (*(volatile uint32_t*)(GIC_CPU_BASE  + 0x004))
#define GICC_IAR        (*(volatile uint32_t*)(GIC_CPU_BASE  + 0x00C))
#define GICC_EOIR       (*(volatile uint32_t*)(GIC_CPU_BASE  + 0x010))

#define TIMER_BASE      0x10011000
#define TIMER_LOAD      (*(volatile uint32_t*)(TIMER_BASE + 0x00))
#define TIMER_CTRL      (*(volatile uint32_t*)(TIMER_BASE + 0x08))
#define TIMER_INTCLR    (*(volatile uint32_t*)(TIMER_BASE + 0x0C))

#ifdef __NEXT_PROC

static void EnableNextProcess(void)
{
        TIMER_INTCLR = 1;
        uint32_t irq = GICC_IAR;
        GICC_EOIR = irq;
}

#endif

#endif
