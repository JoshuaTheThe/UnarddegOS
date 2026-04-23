#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <io.h>

#define IDT_ENTRIES 256

typedef struct __attribute__((__packed__))
{
        uint16_t base_low;
        uint16_t selector;
        uint8_t ist;            /* Interrupt Stack Table (0-7, 0=use normal stack) */
        uint8_t flags;
        uint16_t base_mid;
        uint32_t base_high;
        uint32_t reserved;
} IdtEntry_t;

typedef struct __attribute__((__packed__))
{
        uint16_t limit;
        uint64_t base;
} IdtPtr_t;

void IdtInit(void);
void IdtDefault(void);
void IdtTimer(void);

#endif
