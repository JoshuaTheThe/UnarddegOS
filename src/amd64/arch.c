
#include <arch.h>
#include <stdint.h>
#include <io.h>
#include <panic.h>
#include <_arch.h>
#include <idt.h>
#include <module.h>

enum cpu_vendor
{
        CPU_INTEL,
        CPU_AMD,
        CPU_UNKNOWN
};

#define PIT_FREQUENCY 1193180
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_STATUS 0x61

enum cpu_vendor ArchCPUGetVendor(void)
{
        uint32_t eax, ebx, ecx, edx;
        __asm__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
        if (ebx == 0x756E6547 && edx == 0x49656E69 && ecx == 0x6C65746E)
                return CPU_INTEL;
        if (ebx == 0x68747541 && edx == 0x69746E65 && ecx == 0x444D4163)
                return CPU_AMD;
        return CPU_UNKNOWN;
}

int ArchGetTemperatureMC(void) // millecelsius
{
        enum cpu_vendor vendor = ArchCPUGetVendor();

        if (vendor == CPU_INTEL)
        {
                uint32_t eax, edx;
                __asm__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x1A2));
                uint32_t tjmax = (eax >> 16) & 0xFF;
                __asm__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0x19C));
                uint32_t temp_offset = (eax >> 16) & 0x7F;

                if (temp_offset == 0)
                        return -1;
                return (tjmax - temp_offset) * 1000;
        }

        if (vendor == CPU_AMD)
        {
                // AMD Family 17h+ (Ryzen)
                uint32_t eax, edx;
                __asm__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(0xC0010400));
                if (!(eax & 0x1))
                        return -1;
                uint32_t temp = (eax >> 21) & 0x7FF;
                return temp * 125;
        }

        return -1;
}

void Delay(unsigned long ticks)
{
        extern uint64_t Ticks;
        uint32_t start = Ticks;
        while ((Ticks - start) < ticks)
        {
                __asm volatile("pause");
        }
}

void TimerInit(uint32_t targetFreq)
{
        uint32_t divisor = 1193180 / targetFreq;
        outb(PIT_COMMAND, 0x36);
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void LoadModules(unsigned int magic, unsigned int mb_info_addr)
{
        (void)magic;
        size_t offset = 8;
        uint8_t *base = (void *)((uint64_t)mb_info_addr);
        while (1)
        {
                struct multiboot_tag *tag = (struct multiboot_tag *)(&base[offset]);
                if (tag->type == 0)
                        break;
                if (tag->type == 3)
                {
                        struct multiboot_tag_module *mod = (struct multiboot_tag_module *)tag;
                        uint64_t mod_base = (uint64_t)mod->mod_start;
                        LoadModule((void *)mod_base,
                                   mod->mod_end - mod->mod_start,
                                   mod->cmdline);
                }

                SerialPrint(" [Info] Tag@%x = [%x,%d]\r\n", tag, tag->type, tag->size);

                offset += (tag->size + 7) & ~7;
        }
}

void ArchInitialise(unsigned int magic, unsigned int mb_info_addr)
{
        (void)mb_info_addr;
        if (magic != 0x36d76289)
        {
                Panic(PANIC_INCORRECT_BOOTLOADER);
        }

        IdtInit();
        TimerInit(100);
}

char *ArchIdentify(void)
{
        static char *Architecture = " [Info] ISA: AMD 64 bit x86 Extensions, x86_64, CPU: amd64\n";
        return Architecture;
}

void ArchPause(void)
{
        __asm volatile("pause");
        __asm volatile("hlt");
}

void ArchCli(void)
{
        __asm volatile("cli");
}

void ArchSti(void)
{
        __asm volatile("sti");
}
