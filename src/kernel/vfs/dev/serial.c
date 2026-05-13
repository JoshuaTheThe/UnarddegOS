
#include <vfs/vnode.h>
#include <vfs/dev/serial.h>
#include <drivers/serial.h>
#include <vmem/alloc.h>
#include <panic.h>

// NOTICE - All devices using this
// function will read from the same port
static int SerialReadFunction(void *const Buf,
                               const unsigned long Size,
                               const unsigned long Elements,
                               VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        char *ByteBuf = (char *)Buf;
        unsigned long i;
        // TODO - Rewrite to use interrupts so we aren't blocking
        const unsigned long Bytes = Size * Elements;
        for (i = 0; i < Bytes; ++i)
        {
                char chr = SerialRead();
                if (chr == '\r' && (*(TTYFlags *)Node->DriverData) & TTY_CRNL)
                        chr = '\n';
                if ((*(TTYFlags *)Node->DriverData) & TTY_ECHO)
                {
                        Node->WriteFunction(&chr, 1, 1, Node);
                }
                if (!((*(TTYFlags *)Node->DriverData) & TTY_RAW))
                {
                        if (chr == '\b' && i > 0)
                        {
                                i -= 2;
                                ByteBuf[i] = 0;
                                continue;
                        }
                }
                if (chr == '\n' && (*(TTYFlags *)Node->DriverData) & TTY_COOKED)
                        break;
                ByteBuf[i] = chr;
        }

        return i / Size;
}

// NOTICE - All devices using this
// function will output to the same port
static int SerialWriteFunction(void *const Buf,
                                const unsigned long Size,
                                const unsigned long Elements,
                                VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const unsigned long Bytes = Size * Elements;
        for (unsigned long i = 0; i < Bytes; ++i)
        {
                char chr = ((char *)Buf)[i];
                if (chr == '\n' && *(TTYFlags *)Node->DriverData & TTY_NLCR)
                        SerialPut('\r');
                SerialPut(chr);
        }

        return Bytes;
}

VNode *CreateSerialDevice(const char *const Name, unsigned long NameLength, VNode *Devices)
{
        PanicIfNull(Name);
        PanicIfNull(Devices);
        Trace(SerialInit());
        // panic on fail, no null check required
        VNode *NewSerialDevice = NewVNode(VFS_WRITE | VFS_READ);
        NewSerialDevice->Name.Name   = Name;
        NewSerialDevice->Name.Length = NameLength;
        NewSerialDevice->ReadFunction = SerialReadFunction;
        NewSerialDevice->WriteFunction = SerialWriteFunction;
        NewSerialDevice->DriverData = kalloc(sizeof(TTYFlags));
        *(TTYFlags *)NewSerialDevice->DriverData = TTY_COOKED | TTY_ECHO | TTY_NLCR | TTY_CRNL;
        RegisterChildVNode(Devices, NewSerialDevice);
        NewSerialDevice->WriteFunction(" [Info] Created Serial Virtual Node\r\n",
                                       37,
                                       1,
                                       NewSerialDevice);
        return NewSerialDevice;
}
