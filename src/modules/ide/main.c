#include <ide/main.h>
#include <arch.h>
#include <vfs/vnode.h>
#include <panic.h>
#include <vmem/bumpalloc.h>
#include <string.h>

IDEDriver_t IDEState = {0};
uint8_t package[2048], atapi_packet[2048];

typedef struct
{
        unsigned char sector_buffer[512];
        unsigned long cached_sector;
        int cached_dirty;
} IDECache;

static int IsDrivePresent(unsigned char drive)
{
        if (drive > 3)
                return 0;
        if (IDEState.IDEDev[drive].Reserved == 0)
                return 0;
        unsigned int channel = IDEState.IDEDev[drive].Channel;
        unsigned int slavebit = IDEState.IDEDev[drive].Drive;
        IDEWrite(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4));
        Delay(1);
        uint8_t status = IDERead(channel, ATA_REG_STATUS);
        if (status == 0xFF || status == 0x00)
                return 0;
        return 1;
}

static int IDEReadFunction(void *const Buf,
                           const unsigned long Size,
                           const unsigned long Elements,
                           VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        if (Size == 0 || Elements == 0)
                return 0;

        IDEDev *Self = Node->DriverData;

        // Check if drive is present
        unsigned char drive_num = Self - IDEState.IDEDev;
        if (!IsDrivePresent(drive_num))
        {
                // Drive not present - return zeros
                memset(Buf, 0, Size * Elements);
                Node->FileOffset += Size * Elements;
                return Elements;
        }

        const size_t Bytes = Size * Elements;
        unsigned char *dest = (unsigned char *)Buf;
        unsigned long remaining = Bytes;
        unsigned long current_offset = Node->FileOffset;
        unsigned long bytesPerSector = Self->Type == IDE_ATAPI ? 2048 : 512;

        while (remaining > 0)
        {
                unsigned long sector = current_offset / bytesPerSector;
                unsigned long offset_in_sector = current_offset % bytesPerSector;
                unsigned long bytes_this_iter = bytesPerSector - offset_in_sector;
                if (bytes_this_iter > remaining)
                        bytes_this_iter = remaining;

                unsigned char sector_buffer[bytesPerSector];
                if (sector >= Self->Size)
                {
                        memset(sector_buffer, 0, bytesPerSector);
                }
                else
                {
                        IDEReadSectors(drive_num, 1, sector, 0x10, (uintptr_t)sector_buffer);
                        if (package[0] != 0)
                        {
                                memset(sector_buffer, 0, bytesPerSector);
                                package[0] = 0;
                        }
                }

                for (unsigned long i = 0; i < bytes_this_iter; i++)
                {
                        dest[Bytes - remaining + i] = sector_buffer[offset_in_sector + i];
                }

                current_offset += bytes_this_iter;
                remaining -= bytes_this_iter;
        }

        Node->FileOffset += Bytes;
        return Elements;
}

static int IDEWriteFunction(void *const Buf,
                            const unsigned long Size,
                            const unsigned long Elements,
                            VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        if (Size == 0 || Elements == 0)
                return 0;

        IDEDev *Self = Node->DriverData;
        unsigned char drive_num = Self - IDEState.IDEDev;
        if (!IsDrivePresent(drive_num))
        {
                Node->FileOffset += Size * Elements;
                return Elements;
        }

        if (Self->Type == IDE_ATAPI)
                return -1;

        const size_t Bytes = Size * Elements;
        const unsigned char *src = (const unsigned char *)Buf;
        unsigned long remaining = Bytes;
        unsigned long current_offset = Node->FileOffset;

        while (remaining > 0)
        {
                unsigned long sector = current_offset / 512;
                unsigned long offset_in_sector = current_offset % 512;
                unsigned long bytes_this_iter = 512 - offset_in_sector;
                if (bytes_this_iter > remaining)
                        bytes_this_iter = remaining;

                unsigned char sector_buffer[512];
                if (offset_in_sector != 0 || bytes_this_iter != 512)
                {
                        if (sector < Self->Size)
                        {
                                IDEReadSectors(drive_num, 1, sector, 0x10, (uintptr_t)sector_buffer);
                                if (package[0] != 0)
                                {
                                        memset(sector_buffer, 0, 512);
                                        package[0] = 0;
                                }
                        }
                        else
                        {
                                memset(sector_buffer, 0, 512);
                        }
                }

                for (unsigned long i = 0; i < bytes_this_iter; i++)
                {
                        sector_buffer[offset_in_sector + i] = src[Bytes - remaining + i];
                }

                if (sector < Self->Size)
                {
                        IDEWriteSectors(drive_num, 1, sector, 0x10, (uintptr_t)sector_buffer);
                        if (package[0] != 0)
                        {
                                package[0] = 0;
                        }
                }

                current_offset += bytes_this_iter;
                remaining -= bytes_this_iter;
        }

        Node->FileOffset += Bytes;
        return Elements;
}

void IDEWaitNotBusy(uint8_t channel)
{
        while (IDERead(channel, ATA_REG_STATUS) & ATA_SR_BSY)
                ;
}

void IDEWaitReady(uint8_t channel, uint32_t timeout)
{
        uint8_t status;
        for (uint32_t i = 0; i < timeout; i++)
        {
                status = IDERead(channel, ATA_REG_STATUS);
                if (!(status & ATA_SR_BSY))
                {
                        if (status & ATA_SR_DRQ)
                        {
                                return;
                        }
                }
                Delay(1);
        }
}

void IDEWrite(uint8_t channel, uint8_t reg, uint8_t data)
{
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                outb(IDEState.Channels[channel].base + reg - 0x00, data);
        else if (reg < 0x0C)
                outb(IDEState.Channels[channel].base + reg - 0x06, data);
        else if (reg < 0x0E)
                outb(IDEState.Channels[channel].ctrl + reg - 0x0A, data);
        else if (reg < 0x16)
                outb(IDEState.Channels[channel].bmide + reg - 0x0E, data);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
}

uint8_t IDERead(uint8_t channel, uint8_t reg)
{
        uint8_t result = 0xFF;
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                result = inb(IDEState.Channels[channel].base + reg - 0x00);
        else if (reg < 0x0C)
                result = inb(IDEState.Channels[channel].base + reg - 0x06);
        else if (reg < 0x0E)
                result = inb(IDEState.Channels[channel].ctrl + reg - 0x0A);
        else if (reg < 0x16)
                result = inb(IDEState.Channels[channel].bmide + reg - 0x0E);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
        return result;
}

void IDEReadBuffer(uint8_t channel, uint8_t reg, uint64_t buffer, uint64_t quads)
{
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                insl(IDEState.Channels[channel].base + reg - 0x00, (void *)buffer, quads);
        else if (reg < 0x0C)
                insl(IDEState.Channels[channel].base + reg - 0x06, (void *)buffer, quads);
        else if (reg < 0x0E)
                insl(IDEState.Channels[channel].ctrl + reg - 0x0A, (void *)buffer, quads);
        else if (reg < 0x16)
                insl(IDEState.Channels[channel].bmide + reg - 0x0E, (void *)buffer, quads);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
}

uint8_t IDEPrintErr(uint32_t drive, uint8_t err)
{
        if (err == 0)
                return err;

        SerialPrint(" [Error] IDE:");
        if (err == 1)
        {
                SerialPrint("- Device Fault\n     ");
                err = 19;
        }
        else if (err == 2)
        {
                uint8_t st = IDERead(IDEState.IDEDev[drive].Channel, ATA_REG_ERROR);
                if (st & ATA_ER_AMNF)
                {
                        SerialPrint("- No Address Mark Found\n     ");
                        err = 7;
                }
                if (st & ATA_ER_TK0NF)
                {
                        SerialPrint("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_ABRT)
                {
                        SerialPrint("- Command Aborted\n     ");
                        err = 20;
                }
                if (st & ATA_ER_MCR)
                {
                        SerialPrint("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_IDNF)
                {
                        SerialPrint("- ID mark not Found\n     ");
                        err = 21;
                }
                if (st & ATA_ER_MC)
                {
                        SerialPrint("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_UNC)
                {
                        SerialPrint("- Uncorrectable Data Error\n     ");
                        err = 22;
                }
                if (st & ATA_ER_BBK)
                {
                        SerialPrint("- Bad Sectors\n     ");
                        err = 13;
                }
        }
        else if (err == 3)
        {
                SerialPrint("- Reads Nothing\n     ");
                err = 23;
        }
        else if (err == 4)
        {
                SerialPrint("- Write Protected\n     ");
                err = 8;
        }
        SerialPrint("- [%s %s] %s\n",
                    (const char *[]){"Primary", "Secondary"}[IDEState.IDEDev[drive].Channel],
                    (const char *[]){"Master", "Slave"}[IDEState.IDEDev[drive].Drive],
                    IDEState.IDEDev[drive].Model);

        return err;
}

uint8_t IDEPolling(uint8_t channel, uint32_t advanced_check)
{
        for (int i = 0; i < 4; i++)
                IDERead(channel, ATA_REG_ALTSTATUS);

        while (IDERead(channel, ATA_REG_STATUS) & ATA_SR_BSY)
                ;

        if (advanced_check)
        {
                uint8_t state = IDERead(channel, ATA_REG_STATUS);
                if (state & ATA_SR_ERR)
                        return 2;
                if (state & ATA_SR_DF)
                        return 1;
                if ((state & ATA_SR_DRQ) == 0)
                        return 3;
        }

        return 0;
}

void IDEInitialise(void)
{
        if (!IDEState.Dev)
                return;
        int i, j, k, count = 0;
        IDEState.Channels[ATA_PRIMARY].base = (IDEState.Dev->bar[0] & 0xFFFFFFFC) + 0x1F0 * (!IDEState.Dev->bar[0]);
        IDEState.Channels[ATA_PRIMARY].ctrl = (IDEState.Dev->bar[1] & 0xFFFFFFFC) + 0x3F6 * (!IDEState.Dev->bar[1]);
        IDEState.Channels[ATA_SECONDARY].base = (IDEState.Dev->bar[2] & 0xFFFFFFFC) + 0x170 * (!IDEState.Dev->bar[2]);
        IDEState.Channels[ATA_SECONDARY].ctrl = (IDEState.Dev->bar[3] & 0xFFFFFFFC) + 0x376 * (!IDEState.Dev->bar[3]);
        IDEState.Channels[ATA_PRIMARY].bmide = (IDEState.Dev->bar[4] & 0xFFFFFFFC) + 0;
        IDEState.Channels[ATA_SECONDARY].bmide = (IDEState.Dev->bar[4] & 0xFFFFFFFC) + 8;
        SerialPrint(" [Info] IDE Channels Set\r\n");

        // Disable IRQs - we're using polling only
        IDEWrite(ATA_PRIMARY, ATA_REG_CONTROL, 0x02);
        IDEWrite(ATA_SECONDARY, ATA_REG_CONTROL, 0x02);
        SerialPrint(" [Info] IDE Polling Mode (IRQs Disabled)\r\n");

        for (i = 0; i < 2; i++)
                for (j = 0; j < 2; j++)
                {
                        uint8_t err = 0, type = IDE_ATA, status;
                        IDEState.IDEDev[count].Reserved = 0;

                        IDEWrite(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
                        Delay(4);
                        IDEWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
                        Delay(4);

                        if (IDERead(i, ATA_REG_STATUS) == 0)
                                continue;

                        // Poll for completion
                        uint32_t timeout = 100000;
                        while (timeout--)
                        {
                                status = IDERead(i, ATA_REG_STATUS);
                                if ((status & ATA_SR_ERR))
                                {
                                        err = 1;
                                        break;
                                }
                                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                                        break;
                                Delay(1);
                        }

                        if (err != 0)
                        {
                                uint8_t cl = IDERead(i, ATA_REG_LBA1);
                                uint8_t ch = IDERead(i, ATA_REG_LBA2);

                                if (cl == 0x14 && ch == 0xEB)
                                        type = IDE_ATAPI;
                                else if (cl == 0x69 && ch == 0x96)
                                        type = IDE_ATAPI;
                                else
                                        continue;

                                IDEWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                                Delay(4);
                        }

                        IDEReadBuffer(i, ATA_REG_DATA, (uint64_t)IDEState.Buff, 128);

                        IDEState.IDEDev[count].Reserved = 1;
                        IDEState.IDEDev[count].Type = type;
                        IDEState.IDEDev[count].Channel = i;
                        IDEState.IDEDev[count].Drive = j;
                        IDEState.IDEDev[count].Signature = *((unsigned short *)(&IDEState.Buff[ATA_IDENT_DEVICETYPE]));
                        IDEState.IDEDev[count].Capabilities = *((unsigned short *)(&IDEState.Buff[ATA_IDENT_CAPABILITIES]));
                        IDEState.IDEDev[count].CommandSets = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_COMMANDSETS]));

                        if (IDEState.IDEDev[count].CommandSets & (1 << 26))
                                IDEState.IDEDev[count].Size = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_MAX_LBA_EXT]));
                        else
                                IDEState.IDEDev[count].Size = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_MAX_LBA]));

                        for (k = 0; k < 40; k += 2)
                        {
                                IDEState.IDEDev[count].Model[k] = IDEState.Buff[ATA_IDENT_MODEL + k + 1];
                                IDEState.IDEDev[count].Model[k + 1] = IDEState.Buff[ATA_IDENT_MODEL + k];
                        }
                        IDEState.IDEDev[count].Model[40] = 0;

                        if (count < 4) // simple char name for now
                        {
                                VNode *Drv = NewVNode(VFS_READ | VFS_WRITE);
                                Drv->Name.Name = BumpAllocate(0x03);
                                Drv->Name.Length = 0x3;
                                Drv->WriteFunction = IDEWriteFunction;
                                Drv->ReadFunction = IDEReadFunction;
                                Drv->DriverData = &IDEState.IDEDev[count];
                                ((char *)Drv->Name.Name)[0] = 'h';
                                ((char *)Drv->Name.Name)[1] = 'd';
                                ((char *)Drv->Name.Name)[2] = count + 'a';
                                VNode *Dev = RootVNode()->RelativeFind(RootVNode(), "/dev", 4);
                                RegisterChildVNode(Dev, Drv);
                        }
                        else
                        {
                                SerialPrint(" [Error] IDE: Could not assign drive a name \r\n");
                        }
                        count++;
                }
}

unsigned char IDEAccess(unsigned char direction, unsigned char drive, unsigned int lba,
                        unsigned char numsects, unsigned short selector, unsigned int edi)
{
        unsigned char lba_mode, dma, cmd;
        unsigned char lba_io[6];
        unsigned int channel = IDEState.IDEDev[drive].Channel;
        unsigned int slavebit = IDEState.IDEDev[drive].Drive;
        unsigned int bus = IDEState.Channels[channel].base;
        unsigned int words = 256;
        unsigned short cyl, i;
        unsigned char head, sect, err;

        // Disable IRQs - polling mode
        IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN = 0x02);

        if (lba >= 0x10000000)
        {
                lba_mode = 2;
                lba_io[0] = (lba & 0x000000FF) >> 0;
                lba_io[1] = (lba & 0x0000FF00) >> 8;
                lba_io[2] = (lba & 0x00FF0000) >> 16;
                lba_io[3] = (lba & 0xFF000000) >> 24;
                lba_io[4] = 0;
                lba_io[5] = 0;
                head = 0;
        }
        else if (IDEState.IDEDev[drive].Capabilities & 0x200)
        {
                lba_mode = 1;
                lba_io[0] = (lba & 0x00000FF) >> 0;
                lba_io[1] = (lba & 0x000FF00) >> 8;
                lba_io[2] = (lba & 0x0FF0000) >> 16;
                lba_io[3] = 0;
                lba_io[4] = 0;
                lba_io[5] = 0;
                head = (lba & 0xF000000) >> 24;
        }
        else
        {
                lba_mode = 0;
                sect = (lba % 63) + 1;
                cyl = (lba + 1 - sect) / (16 * 63);
                lba_io[0] = sect;
                lba_io[1] = (cyl >> 0) & 0xFF;
                lba_io[2] = (cyl >> 8) & 0xFF;
                lba_io[3] = 0;
                lba_io[4] = 0;
                lba_io[5] = 0;
                head = (lba + 1 - sect) % (16 * 63) / (63);
        }

        dma = 0;

        while (IDERead(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        {
        }

        if (lba_mode == 0)
                IDEWrite(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head);
        else
                IDEWrite(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head);

        if (lba_mode == 2)
        {
                IDEWrite(channel, ATA_REG_SECCOUNT1, 0);
                IDEWrite(channel, ATA_REG_LBA3, lba_io[3]);
                IDEWrite(channel, ATA_REG_LBA4, lba_io[4]);
                IDEWrite(channel, ATA_REG_LBA5, lba_io[5]);
        }

        IDEWrite(channel, ATA_REG_SECCOUNT0, numsects);
        IDEWrite(channel, ATA_REG_LBA0, lba_io[0]);
        IDEWrite(channel, ATA_REG_LBA1, lba_io[1]);
        IDEWrite(channel, ATA_REG_LBA2, lba_io[2]);

        if (lba_mode == 0 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO;
        if (lba_mode == 1 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO;
        if (lba_mode == 2 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO_EXT;
        if (lba_mode == 0 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA;
        if (lba_mode == 1 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA;
        if (lba_mode == 2 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA_EXT;
        if (lba_mode == 0 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO;
        if (lba_mode == 1 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO;
        if (lba_mode == 2 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO_EXT;
        if (lba_mode == 0 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA;
        if (lba_mode == 1 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA;
        if (lba_mode == 2 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA_EXT;

        IDEWrite(channel, ATA_REG_COMMAND, cmd);

        if (dma)
        {
                if (direction == 0)
                        ;
                else
                        ;
        }
        else if (direction == 0)
        {
                for (i = 0; i < numsects; i++)
                {
                        if ((err = IDEPolling(channel, 1)))
                                return err;
                        __asm volatile("mov %%ax, %%es" : : "a"(selector));
                        __asm volatile("rep insw" : : "c"(words), "d"(bus), "D"(edi));
                        edi += (words * 2);
                }
        }
        else
        {
                for (i = 0; i < numsects; i++)
                {
                        IDEPolling(channel, 0);
                        __asm volatile("mov %%ax, %%ds" ::"a"(selector));
                        __asm volatile("rep outsw" ::"c"(words), "d"(bus), "S"(edi));
                        edi += (words * 2);
                }
                IDEWrite(channel, ATA_REG_COMMAND, (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
                IDEPolling(channel, 0);
        }

        return 0;
}

unsigned char IDEAtaPIRead(unsigned char drive, unsigned int lba, unsigned char numsects,
                           unsigned short selector, unsigned int edi)
{
        unsigned int channel = IDEState.IDEDev[drive].Channel;
        unsigned int slavebit = IDEState.IDEDev[drive].Drive;
        unsigned int bus = IDEState.Channels[channel].base;
        unsigned int words = 1024;
        unsigned char err;
        int i;

        // Disable IRQs - polling mode
        IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN = 0x02);

        atapi_packet[0] = ATAPI_CMD_READ;
        atapi_packet[1] = 0x0;
        atapi_packet[2] = (lba >> 24) & 0xFF;
        atapi_packet[3] = (lba >> 16) & 0xFF;
        atapi_packet[4] = (lba >> 8) & 0xFF;
        atapi_packet[5] = (lba >> 0) & 0xFF;
        atapi_packet[6] = 0x0;
        atapi_packet[7] = 0x0;
        atapi_packet[8] = 0x0;
        atapi_packet[9] = numsects;
        atapi_packet[10] = 0x0;
        atapi_packet[11] = 0x0;

        IDEWrite(channel, ATA_REG_HDDEVSEL, slavebit << 4);

        for (int i = 0; i < 4; i++)
                IDERead(channel, ATA_REG_ALTSTATUS);

        IDEWrite(channel, ATA_REG_FEATURES, 0);
        IDEWrite(channel, ATA_REG_LBA1, (words * 2) & 0xFF);
        IDEWrite(channel, ATA_REG_LBA2, (words * 2) >> 8);

        IDEWrite(channel, ATA_REG_COMMAND, ATA_CMD_PACKET);

        if ((err = IDEPolling(channel, 1)))
                return err;

        __asm volatile("rep outsw" : : "c"(6), "d"(bus), "S"(atapi_packet));

        for (i = 0; i < numsects; i++)
        {
                uint32_t timeout = 100000;
                uint8_t status;
                while (timeout--)
                {
                        status = IDERead(channel, ATA_REG_STATUS);
                        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                                break;
                        Delay(1);
                }

                if ((err = IDEPolling(channel, 1)))
                        return err;

                __asm volatile("mov %%ax, %%es" ::"a"(selector));
                __asm volatile("rep insw" ::"c"(words), "d"(bus), "D"(edi));
                edi += (words * 2);
        }

        uint32_t timeout = 100000;
        while (timeout--)
        {
                uint8_t status = IDERead(channel, ATA_REG_STATUS);
                if (!(status & (ATA_SR_BSY | ATA_SR_DRQ)))
                        break;
                Delay(1);
        }

        return 0;
}

void IDEReadSectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                    unsigned short es, uintptr_t edi)
{
        int i;
        if (!IsDrivePresent(drive))
        {
                package[0] = 0;
                for (unsigned int bytes = 0; bytes < numsects * 512; bytes++)
                {
                        ((unsigned char *)edi)[bytes] = 0;
                }
                return;
        }

        if (drive > 3 || IDEState.IDEDev[drive].Reserved == 0)
                package[0] = 0x1;
        else if (((lba + numsects) > IDEState.IDEDev[drive].Size) && (IDEState.IDEDev[drive].Type == IDE_ATA))
                package[0] = 0x2;
        else
        {
                unsigned char err = 0;
                if (IDEState.IDEDev[drive].Type == IDE_ATA)
                        err = IDEAccess(ATA_READ, drive, lba, numsects, es, edi);
                else if (IDEState.IDEDev[drive].Type == IDE_ATAPI)
                        for (i = 0; i < numsects; i++)
                                err = IDEAtaPIRead(drive, lba + i, 1, es, edi + (i * 2048));
                package[0] = IDEPrintErr(drive, err);
        }
}

void IDEWriteSectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                     unsigned short es, uintptr_t edi)
{
        if (!IsDrivePresent(drive))
        {
                package[0] = 0;
                return;
        }

        if (drive > 3 || IDEState.IDEDev[drive].Reserved == 0)
                package[0] = 0x1;
        else if (((lba + numsects) > IDEState.IDEDev[drive].Size) && (IDEState.IDEDev[drive].Type == IDE_ATA))
                package[0] = 0x2;
        else
        {
                unsigned char err = 0;
                if (IDEState.IDEDev[drive].Type == IDE_ATA)
                        err = IDEAccess(ATA_WRITE, drive, lba, numsects, es, edi);
                else if (IDEState.IDEDev[drive].Type == IDE_ATAPI)
                        err = 4;
                package[0] = IDEPrintErr(drive, err);
        }
}

void IDEFind(PCI *Pci, size_t Index)
{
        IDEState.Dev = PCIGetOriginalDevice(Pci, Index);
        if (!IDEState.Dev || IDEState.Dev->class_id != 0x01 || IDEState.Dev->subclass_id != 0x01)
        {
                SerialPrint(" [Error] Could not find IDE Device\r\n");
                return;
        }

        SerialPrint(" [Info] IDE Device found\r\n");
        IDEInitialise();
}

int Init(void)
{
        VNode *Pci = RootVNode()->RelativeFind(RootVNode(), "/dev/pci", 8);
        if (!Pci)
        {
                SerialPrint(" [Error] IDE Module could not find PCI\r\n");
                return -1;
        }

        PCI *PciData = Pci->DriverData;
        PanicIfNull(PciData);
        size_t count = 0;
        for (size_t i = 0; i < PciData->Count; ++i)
        {
                if (PciData->Dev[i].class_id == 0x01 && PciData->Dev[i].subclass_id == 0x01)
                {
                        IDEFind(PciData, i);
                        count++;
                }
        }

        return 0;
}
