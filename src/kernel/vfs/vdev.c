
#include <vfs/vdev.h>
#include <vfs/dev/serial.h>
#include <vfs/dev/null.h>
#include <vfs/dev/random.h>
#include <panic.h>

void VFSCreateDevices(void)
{
        // Initialise Root
        VNodeDefault(RootVNode());
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
        Trace((void)CreateNullDevice  ("null", 4, Node));
        Trace((void)CreateSerialDevice("tty0", 4, Node));
        Trace((void)CreateRandomDevice("random", 6, Node));

        VNode *Mnt = NewVNode(VFS_SYSTEM);
        Mnt->Name.Name   = "mnt";
        Mnt->Name.Length = 3;
        RegisterChildVNode(RootVNode(), Mnt);
}
