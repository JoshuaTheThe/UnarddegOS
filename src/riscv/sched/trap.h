#ifndef TRAP_H
#define TRAP_H

#define CLINT_BASE      0x02000000UL
#define CLINT_MTIMECMP  (*(volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME     (*(volatile uint64_t*)(CLINT_BASE + 0xBFF8))

#define TIMER_INTERVAL  10000

static void EnableNextProcess(void)
{
        CLINT_MTIMECMP = CLINT_MTIME + TIMER_INTERVAL;
}

#endif
