#include <idt.h>
#include <arch.h>

static IdtEntry_t idt[256];
static IdtPtr_t idtp;

void IdtSetEntry(uint8_t n, void *handler, IdtEntry_t *idtEntries)
{
        idtEntries[n].base_low = (uint32_t)handler & 0xFFFF;
        idtEntries[n].base_high = ((uint32_t)handler >> 16) & 0xFFFF;
        idtEntries[n].selector = 8;
        idtEntries[n].always_0 = 0;
        idtEntries[n].flags = 0x8E;
}

void IdtInit(void)
{
        ArchCli();
        for (uint64_t i = 0; i < IDT_ENTRIES; ++i)
        {
                IdtSetEntry((uint8_t)i, (void *)IdtDefault, idt);
        }

        IdtSetEntry(0x20, (void *)IdtTimer, idt);
        idtp.limit = (sizeof(IdtEntry_t) * IDT_ENTRIES) - 1;
        idtp.base = (uint32_t)idt;
        asm volatile ("lidt (%0)" : : "r"(&idtp));

        /* PIC Remap */
        outb(0x20, 0x11);
        outb(0xA0, 0x11);
        outb(0x21, 0x20);
        outb(0xA1, 0x28);
        outb(0x21, 0x04);
        outb(0xA1, 0x02);
        outb(0x21, 0x01);
        outb(0xA1, 0x01);
        outb(0x21, 0xFA);
        outb(0xA1, 0x3F);
}

