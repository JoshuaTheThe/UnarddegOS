
#include <serial.h>
#include <panic.h>
#include <vmem/bumpalloc.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <vfs/vdev.h>

void kmain(void)
{
        Trace(ArchInitialise());
        Trace(SerialInit());
        Trace(SerialPrint("%s\r\n", ArchIdentify()));
        Trace(VFSCreateDevices());
        Trace(VNListTree(RootVNode(), 1));
        Panic(PANIC_TODO);
        while(1);
}
