
#include <arch.h>
#include <stdint.h>
#include <io.h>
#include <panic.h>
#include <_arch.h>
#include <module.h>
#include <idt.h>

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

void LoadModules(unsigned int a, unsigned int b)
{
        (void)a;
        (void)b;
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
