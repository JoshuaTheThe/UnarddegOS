
#include <arch.h>
#include <stdint.h>
#include <io.h>
#include <gdt.h>
#include <idt.h>
#include <panic.h>
#include <_arch.h>
#include <module.h>

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_STATUS 0x61

void Delay(unsigned long ticks)
{
        extern uint64_t Ticks;
        uint32_t start = Ticks;
        while ((Ticks - start) < ticks)
        { __asm volatile("pause"); }
}

void TimerInit(uint32_t targetFreq)
{
        uint32_t divisor = 1193180 / targetFreq;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void ArchInitialise(unsigned int magic, unsigned int mb_info_addr)
{
        (void)mb_info_addr;
        if (magic != 0x36d76289)
        {
                Panic(PANIC_INCORRECT_BOOTLOADER);
        }

        GdtInit();
        IdtInit();
        TimerInit(100);
}

void LoadModules(unsigned int magic, unsigned int mb_info_addr)
{
        (void)magic;
        unsigned int offset = 8;
        struct multiboot_tag *tag;
        while (1)
        {
                tag = (struct multiboot_tag *)(mb_info_addr + offset);

                if (tag->type == 0)
                        break;
                if (tag->type == 3)
                {
                        struct multiboot_tag_module *mod = (struct multiboot_tag_module *)tag;
                        LoadModule((void*)mod->mod_start,
                                   mod->mod_end - mod->mod_start,
                                   mod->cmdline);
                }

                offset += (tag->size + 7) & ~7;
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
