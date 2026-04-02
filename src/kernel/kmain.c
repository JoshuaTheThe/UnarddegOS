
#include <drivers/serial.h>
#include <panic.h>
#include <vmem/bumpalloc.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <vfs/vdev.h>
#include <string.h>
#include <sched/core.h>

void kmain(void)
{
        static char Message[] = " [Info] Found tty0 VNode Successfully\n";
        static char InputString[16];
        VNode *tty0;
        Trace(ArchInitialise());
        Trace(SchedInit());
        Trace(VFSCreateDevices());
        Trace(tty0 = RootVNode()->RelativeFind(RootVNode(), "/dev/tty0", 9));
        tty0->WriteFunction(ArchIdentify(), strnlen(ArchIdentify(), 64), 1, tty0);
        tty0->WriteFunction(Message, sizeof(Message), 1, tty0);
        tty0->ReadFunction(InputString, sizeof(InputString) - 1, 1, tty0);
        Panic(PANIC_TODO);
        while(1);
}
