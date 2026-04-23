
#include <vfs/dev/null.h>
#include <panic.h>
#include <string.h>

static int NullReadFunction(void *const Buf,
                            const unsigned long Size,
                            const unsigned long Elements,
                            VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const unsigned long Bytes = Size * Elements;
        memset(Buf, 0, Bytes);
        return Elements;
}

static int NullWriteFunction(void *const Buf,
                             const unsigned long Size,
                             const unsigned long Elements,
                             VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        (void)Size;
        (void)Elements;
        return Elements;
}

VNode *CreateNullDevice(const char *const Name, unsigned long NameLength, VNode *Devices)
{
        PanicIfNull(Name);
        PanicIfNull(Devices);
        // panic on fail, no null check required
        VNode *NewNullDevice = NewVNode(VFS_WRITE | VFS_READ);
        NewNullDevice->Name.Name      = Name;
        NewNullDevice->Name.Length    = NameLength;
        NewNullDevice->ReadFunction   = NullReadFunction;
        NewNullDevice->WriteFunction  = NullWriteFunction;
        RegisterChildVNode(Devices, NewNullDevice);
        return NewNullDevice;
}

