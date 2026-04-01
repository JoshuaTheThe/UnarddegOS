
#include <vfs/vnode.h>
#include <vfs/dev/serial.h>
#include <common/serial.h>
#include <panic.h>

// NOTICE - All devices using this
// function will read from the same port
static void SerialReadFunction(void *const Buf,
                               const unsigned long Size,
                               const unsigned long Elements,
                               VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        char *ByteBuf = (char *)Buf;
        // TODO - Rewrite to use interrupts so we aren't blocking
        const unsigned long Bytes = Size * Elements;
        for (unsigned long i = 0; i < Bytes; ++i)
        {
                ByteBuf[i] = SerialRead();
        }
}

// NOTICE - All devices using this
// function will output to the same port
static void SerialWriteFunction(void *const Buf,
                                const unsigned long Size,
                                const unsigned long Elements,
                                VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const unsigned long Bytes = Size * Elements;
        for (unsigned long i = 0; i < Bytes; ++i)
        {
                SerialPut(((char *)Buf)[i]);
        }
}

VNode *CreateSerialDevice(const char *const Name, unsigned long NameLength, VNode *Devices)
{
        PanicIfNull(Name);
        PanicIfNull(Devices);
        // panic on fail, no null check required
        VNode *NewSerialDevice = NewVNode(VFS_WRITE | VFS_READ);
        NewSerialDevice->Name.Name   = Name;
        NewSerialDevice->Name.Length = NameLength;
        NewSerialDevice->ReadFunction = SerialReadFunction;
        NewSerialDevice->WriteFunction = SerialWriteFunction;
        RegisterChildVNode(Devices, NewSerialDevice);
        NewSerialDevice->WriteFunction(" [Info] Created Serial Virtual Node\r\n",
                                       38,
                                       1,
                                       NewSerialDevice);
        return NewSerialDevice;
}
