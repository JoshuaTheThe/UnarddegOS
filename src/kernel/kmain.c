
#include <drivers/serial.h>
#include <panic.h>
#include <vmem/bumpalloc.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <vfs/vdev.h>
#include <vfs/vfd.h>
#include <string.h>
#include <sched/core.h>

void kmain(void)
{
        static char Message[] = " [Info] Found tty0 VNode Successfully\n";
        static char InputString[16];
        ArchInitialise();
        VFSCreateDevices();
        SchedulerInitialise();
        FileDescriptor File = open("/dev/tty0", 0);
        ArchSti();
        write(File, ArchIdentify(), strnlen(ArchIdentify(), 64));
        write(File, Message, sizeof(Message));
        read(File, InputString, sizeof(InputString) - 1);
        VNListTree(RootVNode(), 1);
        Panic(PANIC_TODO);
        while(1);
}
