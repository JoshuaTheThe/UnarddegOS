
#include <vfs/vdev.h>
#include <vfs/dev/serial.h>
#include <panic.h>

void VFSCreateDevices(void)
{
        // Initialise Root
        RootVNode()->Name.Name = "root";
        RootVNode()->Name.Length = 4;

        VNode *Node;
        
        // Create /dev
        // panic on fail, trace is macro so we know where we came from
        Trace(Node = NewVNode(VFS_READ | VFS_WRITE));
        Trace((void)RegisterChildVNode(RootVNode(), Node));
        Node->Name.Name   = "dev";
        Node->Name.Length = 3;

        // create devices
        Trace((void)CreateSerialDevice("tty0", 4, Node));
}
