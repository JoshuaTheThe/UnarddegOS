#define FUNCTION
#include <pci/main.h>
#include <vfs/vnode.h>
#include <drivers/serial.h>
#include <vmem/bumpalloc.h>
#include <panic.h>
#include <string.h>


const char *PCIClassToString(PCI *Pci, uint8_t class_id, uint8_t subclass_id)
{
        (void)Pci;
        switch (class_id)
        {
        case 0x00:
                if (subclass_id == 0x01)
                        return "VGA Compatible (Legacy)";
                return "Unclassified";

        case 0x01:
                switch (subclass_id)
                {
                case 0x00:
                        return "SCSI Controller";
                case 0x01:
                        return "IDE Controller";
                case 0x02:
                        return "Floppy Disk Controller";
                case 0x03:
                        return "IPI Bus Controller";
                case 0x04:
                        return "RAID Controller";
                case 0x05:
                        return "ATA Controller";
                case 0x06:
                        return "SATA Controller";
                case 0x07:
                        return "SAS Controller";
                case 0x80:
                        return "Other Mass Storage Controller";
                default:
                        return "Unknown Storage Controller";
                }

        case 0x02:
                switch (subclass_id)
                {
                case 0x00:
                        return "Ethernet Controller";
                case 0x01:
                        return "Token Ring Controller";
                case 0x02:
                        return "FDDI Controller";
                case 0x03:
                        return "ATM Controller";
                case 0x04:
                        return "ISDN Controller";
                case 0x05:
                        return "WorldFip Controller";
                case 0x06:
                        return "PICMG Controller";
                case 0x80:
                        return "Other Network Controller";
                default:
                        return "Unknown Network Controller";
                }

        case 0x03:
                switch (subclass_id)
                {
                case 0x00:
                        return "VGA Compatible Controller";
                case 0x01:
                        return "XGA Controller";
                case 0x02:
                        return "3D Controller (Not VGA-Compatible)";
                case 0x80:
                        return "Other Display Controller";
                default:
                        return "Unknown Display Controller";
                }

        case 0x06:
                switch (subclass_id)
                {
                case 0x00:
                        return "Host Bridge";
                case 0x01:
                        return "ISA Bridge";
                case 0x02:
                        return "EISA Bridge";
                case 0x03:
                        return "MCA Bridge";
                case 0x04:
                        return "PCI-to-PCI Bridge";
                case 0x05:
                        return "PCMCIA Bridge";
                case 0x06:
                        return "NuBus Bridge";
                case 0x07:
                        return "CardBus Bridge";
                case 0x08:
                        return "RACEway Bridge";
                case 0x09:
                        return "PCI-to-PCI Bridge (Semi-transparent)";
                case 0x0A:
                        return "InfiniBand-to-PCI Host Bridge";
                case 0x80:
                        return "Other Bridge";
                default:
                        return "Unknown Bridge";
                }

        case 0x0C:
                switch (subclass_id)
                {
                case 0x00:
                        return "FireWire (IEEE 1394) Controller";
                case 0x01:
                        return "ACCESS Bus";
                case 0x02:
                        return "SSA";
                case 0x03:
                        return "USB Controller";
                case 0x04:
                        return "Fibre Channel";
                case 0x05:
                        return "SMBus";
                case 0x06:
                        return "InfiniBand";
                case 0x07:
                        return "IPMI Interface";
                case 0x08:
                        return "SERCOS Interface";
                case 0x09:
                        return "CANbus Controller";
                default:
                        return "Unknown Serial Bus Controller";
                }

        default:
                return "Unknown Device";
        }
}

static inline uint32_t PCIConfigReadDword(PCI *Pci, uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
        (void)Pci;
        uint32_t address = ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                           ((uint32_t)offset & 0xFC) | 0x80000000;
        outl(PCI_CONFIG_ADDRESS, address);
        return inl(PCI_CONFIG_DATA);
}

static inline uint16_t PCIConfigReadWord(PCI *Pci, uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
        (void)Pci;
        uint32_t data = PCIConfigReadDword(Pci, bus, slot, func, offset);
        return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

static inline uint8_t PCIConfigReadByte(PCI *Pci, uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
        uint32_t data = PCIConfigReadDword(Pci, bus, slot, func, offset);
        return (uint8_t)((data >> ((offset & 3) * 8)) & 0xFF);
}

static inline int PCIDeviceExists(PCI *Pci, uint16_t bus, uint8_t slot, uint8_t function)
{
        return PCIConfigReadWord(Pci, bus, slot, function, 0x00) != 0xFFFF;
}

static inline void PCIReadDeviceInfo(PCI *Pci, PCIDEV *dev, uint16_t bus, uint8_t slot, uint8_t func)
{
        (void)Pci;
        dev->bus = (uint8_t)bus;
        dev->slot = slot;
        dev->function = func;
        dev->vendor_id = PCIConfigReadWord(Pci, bus, slot, func, 0x00);
        dev->device_id = PCIConfigReadWord(Pci, bus, slot, func, 0x02);
        dev->revision = PCIConfigReadByte(Pci, bus, slot, func, 0x08);
        dev->prog_if = PCIConfigReadByte(Pci, bus, slot, func, 0x09);
        dev->subclass_id = PCIConfigReadByte(Pci, bus, slot, func, 0x0A);
        dev->class_id = PCIConfigReadByte(Pci, bus, slot, func, 0x0B);
        dev->header_type = PCIConfigReadByte(Pci, bus, slot, func, 0x0E);
        dev->irq_line = PCIConfigReadByte(Pci, bus, slot, func, 0x3C);

        for (int i = 0; i < 6; i++)
        {
                uint32_t bar_val = PCIConfigReadDword(Pci, bus, slot, func, (uint8_t)(0x10 + i * 4));
                dev->bar[i] = bar_val;
        }
}

void PCIEnumeratePci(PCI *Pci, void (*OnDeviceFound)(PCI *Pci, PCIDEV *))
{
        for (uint16_t bus = 0; bus < 256; bus++)
        {
                for (uint8_t slot = 0; slot < 32; slot++)
                {
                        if (!PCIDeviceExists(Pci, bus, slot, 0))
                                continue;

                        uint8_t header_type = PCIConfigReadByte(Pci, bus, slot, 0, 0x0E);
                        uint8_t func_limit = (header_type & 0x80) ? 8 : 1;

                        for (uint8_t func = 0; func < func_limit; func++)
                        {
                                if (!PCIDeviceExists(Pci, bus, slot, func))
                                        continue;

                                PCIDEV dev;
                                PCIReadDeviceInfo(Pci, &dev, bus, slot, func);
                                OnDeviceFound(Pci, &dev);
                        }
                }
        }
}

void PCIRegister(PCI *Pci, PCIDEV *Dev)
{
        Pci->Dev[Pci->Count++] = *Dev;
        SerialPrint(" [Info] Found PCI (%x) device of type %s\r\n", Pci, PCIClassToString(Pci, Dev->class_id, Dev->subclass_id));
}

PCIDEV *PCIFindOfType(PCI *Pci, uint8_t class_id, uint8_t subclass_id)
{
        for (int i = 0; i < MAX_DEVICES; ++i)
        {
                if (Pci->Dev[i].class_id == class_id && Pci->Dev[i].subclass_id == subclass_id)
                {
                        return &Pci->Dev[i];
                }
        }
        return NULL;
}

static int PCIReadFunction(void *const Buf,
                           const unsigned long Size,
                           const unsigned long Elements,
                           VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const size_t Bytes = Size * Elements;
        if (Bytes <= sizeof(PCI))
        {
                memcpy(Buf, Node->DriverData, Bytes);
        }
        else if (Bytes > sizeof(PCI))
        {
                memcpy(Buf, Node->DriverData, sizeof(PCI));
        }
        return Elements;
}

static int PCIWriteFunction(void *const Buf,
                            const unsigned long Size,
                            const unsigned long Elements,
                            VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const size_t Bytes = Size * Elements;
        if (Bytes <= sizeof(PCI))
        {
                memcpy(Node->DriverData, Buf, Bytes);
        }
        else if (Bytes > sizeof(PCI))
        {
                memcpy(Node->DriverData, Buf, sizeof(PCI));
        }
        return Elements;
}

int Init(void)
{
        PCI   *Pci = BumpAllocate(sizeof(PCI));
        VNode *DevFile = RootVNode()->RelativeFind(RootVNode(), "dev", 3);
        if (!DevFile)
                return -1;
        VNode *PciFile = NewVNode(VFS_READ | VFS_WRITE);
        PciFile->Name.Name     = "pci";
        PciFile->Name.Length   = 3;
        PciFile->DriverData    = Pci;
        PciFile->WriteFunction = PCIWriteFunction;
        PciFile->ReadFunction  = PCIReadFunction;
        RegisterChildVNode(DevFile, PciFile);
        PCIEnumeratePci(Pci, PCIRegister);
        return Pci->Count;
}
