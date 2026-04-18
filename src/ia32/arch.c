
#include <arch.h>
#include <stdint.h>
#include <io.h>
#include <gdt.h>
#include <idt.h>
#include <panic.h>
#include <_arch.h>

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_STATUS 0x61

void TimerInit(uint32_t targetFreq)
{
        uint32_t divisor = 1193180 / targetFreq;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void ArchInitialise(unsigned int a, unsigned int b)
{
        if (a != 0x36d76289)
        {
                Panic(PANIC_INCORRECT_BOOTLOADER);
        }
        (void)a;
        (void)b;
        GdtInit();
        IdtInit();
        TimerInit(100);

        unsigned int offset = 8;
        while (1)
        {
                struct multiboot_tag *tag = (struct multiboot_tag *)(b + offset);
                if (tag->type == 0)
                        break; // End tag
                offset += (tag->size + 7) & ~7; // Align
        }
}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: Intel Architecture / 32,x86, CPU: i386\n";
        return Architecture;
}

void ArchCli(void)
{
        __asm volatile("cli");
}

void ArchSti(void)
{
        __asm volatile("sti");
}
