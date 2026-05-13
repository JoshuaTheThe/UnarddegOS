
#include <drivers/serial.h>
#include <panic.h>
#include <vmem/bitmap.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <vfs/vdev.h>
#include <vfs/vfd.h>
#include <string.h>
#include <sched/core.h>
#ifdef MODULE
#include <module.h>
#endif

void kmain(unsigned int a, unsigned int b)
{
        static char Message[] = " [Info] Found tty0 VNode Successfully\n";
        static char InputString[16];
        VFSCreateDevices();
        ArchInitialise(a, b);
        SchedulerInitialise();
        ArchSti();
        #ifdef MODULE
        SerialPrint(" [Info] Loading Modules...\r\n");
        LoadModules(a, b);
        #endif
        FileDescriptor File = open("/dev/tty0", 0);
        write(File, ArchIdentify(), strnlen(ArchIdentify(), 64));
        write(File, Message, sizeof(Message));
        read(File, InputString, sizeof(InputString) - 1);
        close(File);
        VNListTree(RootVNode(), 1);
        SerialPrint(" [Mem] %dkb in use (%d/%d)\r\n",
                    (TOTAL_BITMAP - MStat()) * PAGE_SIZE / 1024,
                    TOTAL_BITMAP - MStat(), TOTAL_BITMAP);

        size_t SampleDelay = 1000, WaitTimeRemaining = 0;
        while(1)
        {
                if (WaitTimeRemaining == 0 && SampleDelay > 0)
                {
                        #ifdef HAS_TEMPERATURE
                        long Temperature = ArchGetTemperatureMC(); // return -1 if NaN or N/A
                        #else
                        long Temperature = -1;
                        #endif
                        // -1 is invalid so uh
                        if (Temperature == -1) SampleDelay = -1; // we dont care if non present

                        if (Temperature < (60*1000))
                                SerialPrint(" [Info] Temperature (%d) is normal\r\n", Temperature / 1000);
                        else if (Temperature > (85*1000))
                        {
                                ArchCli();
                                SerialPrint(" [Info] Temperature (%d) is critical, aborting\r\n", Temperature / 1000);
                                Panic(PANIC_OVERHEAT);
                        }
                        else if (Temperature > (80*1000))
                                SerialPrint(" [Info] Temperature (%d) is... high.. hmm..\r\n", Temperature / 1000);
                        else if (Temperature > (70*1000))
                                SerialPrint(" [Info] Temperature (%d) surpassed expected temperatures\r\n", Temperature / 1000);
                        else if (Temperature > (60*1000))
                                SerialPrint(" [Info] Temperature (%d) implies inefficiency\r\n", Temperature / 1000);
                        WaitTimeRemaining = SampleDelay;
                }
                else if (SampleDelay > 0)
                        WaitTimeRemaining -= 1;
                ArchPause(); // wait for interrupt
        }

        Panic(PANIC_TODO);
}
