#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define MAX_DEVICES 128

typedef struct
{
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        uint16_t vendor_id;
        uint16_t device_id;
        uint8_t class_id;
        uint8_t subclass_id;
        uint8_t prog_if;
        uint8_t revision;
        uint8_t header_type;
        uint8_t irq_line;
        uint32_t bar[6];
} PCIDEV;

typedef struct
{
        PCIDEV   Dev[MAX_DEVICES];
        uint32_t Count;
} PCI;

#ifdef FUNCTION

size_t PCIGetDevices(PCI *Pci, PCIDEV *destination, size_t start, size_t end);
const char *PCIClassToString(PCI *Pci, uint8_t class_id, uint8_t subclass_id);
void PCIEnumerateDevices(PCI *Pci, void (*on_device_found)(PCI *Pci, PCIDEV *));
void PCIDisplayDeviceInfo(PCI *Pci, PCIDEV *dev);
void PCIRegister(PCI *Pci, PCIDEV *dev);
PCIDEV *PCIFindOfType(PCI *Pci, uint8_t class_id, uint8_t subclass_id);

#else

__attribute__((__used__))
static size_t PCIGetPci(PCI *Pci, PCIDEV *destination, size_t start, size_t end)
{
        size_t count = 0;
        for (size_t i = start; i <= end && i < Pci->Count; ++i)
        {
                destination[count] = Pci->Dev[i];
                count++;
        }
        return count;
}

static PCIDEV *PCIGetOriginalDevice(PCI *Pci, size_t Index)
{
        if (Index >= MAX_DEVICES)
                return NULL;
        return &Pci->Dev[Index];
}

#endif

#endif // PCI_H
