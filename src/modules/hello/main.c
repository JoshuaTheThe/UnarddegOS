#include <drivers/serial.h>
#include <sched/core.h>
#include <vmem/alloc.h>
#include <arch.h>

void Hello(void)
{
        ArchSti();
        for (int i = 0; i < 64; ++i)
                SerialPrint(" [Proc] Hello, World! x%d\r\n", i);
        while(1);
}

int Init(void)
{
        TaskRegisters InitialState = {0}; // ia32 only
        InitialState.Registers[4]  = (uint32_t)&((uint32_t *)kalloc(1024*4))[1020]; // esp
        InitialState.Registers[8]  = (uint32_t)&Hello; // eip
        InitialState.Registers[9]  = 0x08;  // cs
        InitialState.Registers[10] = 0x10;  // ds
        InitialState.Registers[11] = 0x10;  // es
        InitialState.Registers[12] = 0x10;  // ss
        InitialState.Registers[13] = 0x10;  // fs
        InitialState.Registers[14] = 0x10;  // gs
        InitialState.Registers[15] = 0x3202; // I | alw1
        SchedulerCreateProc(InitialState);
        return 0;
}
