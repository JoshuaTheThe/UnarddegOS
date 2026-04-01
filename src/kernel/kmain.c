
#include <serial.h>
#include <panic.h>
#include <vmem/bumpalloc.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <vfs/vdev.h>

void kmain(void)
{
        static char Message[] = " [Info] Found tty0 VNode Successfully\r\n";
        VNode *tty0;
        Trace(ArchInitialise());
        Trace(SerialInit());
        Trace(SerialPrint("%s\r\n", ArchIdentify()));
        Trace(VFSCreateDevices());
        Trace(tty0 = RootVNode()->RelativeFind(RootVNode(), "/dev/tty0", 9));
        tty0->WriteFunction(Message, sizeof(Message), 1, tty0);
        Panic(PANIC_TODO);
        while(1);
}
