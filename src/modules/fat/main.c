#include <fat/main.h>
#include <vfs/vnode.h>
#include <vfs/vfd.h>
#include <vmem/alloc.h>
#include <string.h>

void ReadFrom(size_t Offset, void *Buf, size_t Bytes, VNode *Drive)
{
        Drive->FileOffset = Offset;
        Drive->ReadFunction(Buf, Bytes, 1, Drive);
}

void WriteTo(size_t Offset, void *Buf, size_t Bytes, VNode *Drive)
{
        Drive->FileOffset = Offset;
        Drive->WriteFunction(Buf, Bytes, 1, Drive);
}

static inline size_t FatSectorToOffset(FATVolume *vol, DWORD sector)
{
        return (vol->PartitionLBA + sector) * vol->Boot.bpb.BytesPerSector;
}

DWORD FatParseMBR(VNode *Drive)
{
        MBR mbr;
        ReadFrom(0, &mbr, sizeof(mbr), Drive);

        if (mbr.Signature != 0xAA55)
                return 0;

        for (int i = 0; i < 4; i++)
        {
                BYTE type = mbr.Partitions[i].Type;
                if (type == 0x01 || type == 0x04 || type == 0x06 ||
                    type == 0x0B || type == 0x0C || type == 0x0E)
                        return mbr.Partitions[i].LBAStart;
        }

        return 0;
}

FATVolume FatMount(VNode *Drive)
{
        FATVolume vol;
        vol.Drive        = Drive;
        vol.PartitionLBA = FatParseMBR(Drive);
        ReadFrom(vol.PartitionLBA * 512, &vol.Boot, sizeof(vol.Boot), Drive);
        return vol;
}

DWORD FatTotalSectors(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        return (Bt->bpb.TotalSectorsInVolume) ? Bt->bpb.TotalSectorsInVolume : Bt->bpb.ExtendedSectorCount;
}

DWORD FatSize(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        return (Bt->bpb.SectorsPerFAT) ? Bt->bpb.SectorsPerFAT : Bt->as.fat32.SectorsPerFAT;
}

DWORD FatRootDirSize(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        return ((Bt->bpb.RootDirectoryEntries * sizeof(FATDirectory)) + (Bt->bpb.BytesPerSector - 1)) / Bt->bpb.BytesPerSector;
}

DWORD FatFirstData(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        return Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(vol)) + FatRootDirSize(vol);
}

DWORD FatFirstFat(FATVolume *vol)
{
        return vol->Boot.bpb.ReservedSectors;
}

DWORD FatDataCount(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        return FatTotalSectors(vol) - (Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(vol)) + FatRootDirSize(vol));
}

DWORD FatClusterCount(FATVolume *vol)
{
        return FatDataCount(vol) / vol->Boot.bpb.SectorsPerCluster;
}

FATType FatIdentify(FATVolume *vol)
{
        FATBootSector *Bt = &vol->Boot;
        DWORD TotalClusters = FatClusterCount(vol);
        if (TotalClusters < 4085 &&
            (Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
             Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B))
                return FAT_12;
        else if (TotalClusters < 65525 &&
                 (Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
                  Bt->as.fat16.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B))
                return FAT_16;
        else if (Bt->as.fat32.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_A ||
                 Bt->as.fat32.Signature == FAT_EXT_BOOT_RECORD_16_SIGN_B)
                return FAT_32;
        return FAT_UNKNOWN;
}

DWORD FatFirstRoot(FATVolume *vol)
{
        return FatFirstData(vol) - FatRootDirSize(vol);
}

DWORD FatRootCluster(FATVolume *vol)
{
        return vol->Boot.as.fat32.RootCluster;
}

DWORD FatFirstSectorForCluster(FATVolume *vol, DWORD Cluster)
{
        return ((Cluster - 2) * vol->Boot.bpb.SectorsPerCluster) + FatFirstData(vol);
}

void FatConvertPaddedToNull(char *X, size_t L)
{
        for (size_t i = 0; i < L; ++i)
                X[i] = (X[i] == ' ') ? '\0' : X[i];
}

DWORD FatNextCluster32(FATVolume *vol, DWORD cluster)
{
        FATBootSector *bt = &vol->Boot;
        DWORD fatOffset = cluster * 4;
        DWORD fatSector = FatFirstFat(vol) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        ReadFrom(FatSectorToOffset(vol, fatSector), sector, bt->bpb.BytesPerSector, vol->Drive);
        DWORD entry = *(DWORD *)&sector[entOffset];
        return entry & 0x0FFFFFFF;
}

WORD FatNextCluster16(FATVolume *vol, WORD cluster)
{
        FATBootSector *bt = &vol->Boot;
        DWORD fatOffset = cluster * 2;
        DWORD fatSector = FatFirstFat(vol) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        ReadFrom(FatSectorToOffset(vol, fatSector), sector, bt->bpb.BytesPerSector, vol->Drive);
        return *(WORD *)&sector[entOffset];
}

WORD FatNextCluster12(FATVolume *vol, WORD cluster)
{
        FATBootSector *bt = &vol->Boot;
        DWORD fatOffset = cluster + (cluster / 2);
        DWORD fatSector = FatFirstFat(vol) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        ReadFrom(FatSectorToOffset(vol, fatSector), sector, bt->bpb.BytesPerSector, vol->Drive);
        DWORD entry = *(DWORD *)&sector[entOffset];
        if (cluster & 1)
                entry >>= 4;
        else
                entry &= 0x0FFF;
        return entry;
}

DWORD FatNextCluster(FATVolume *vol, DWORD cluster)
{
        switch (FatIdentify(vol))
        {
        case FAT_UNKNOWN:
                return 0xFFFFFFFF;
        case FAT_12:
                return FatNextCluster12(vol, cluster);
        case FAT_16:
                return FatNextCluster16(vol, cluster);
        case FAT_32:
                return FatNextCluster32(vol, cluster);
        default:
                return 0xFFFFFFFF;
        }
}

FATFileLocation FatLocateInDir(BYTE Name[8], BYTE Ext[3], FATVolume *vol, FATDirectory *Parent)
{
        if (Parent && !(Parent->Flags & FAT_DIRECTORY))
                return (FATFileLocation){0};

        FATBootSector *bt = &vol->Boot;
        DWORD cluster = Parent ? (Parent->EntryFirstClusterHigh << 16 | Parent->EntryFirstClusterLow) : FatRootCluster(vol);

        const size_t bytes   = bt->bpb.BytesPerSector;
        const size_t entries = bytes / sizeof(FATDirectory);

        FATDirectory Directory[entries];
        FATFileLocation Result = {0};
        BYTE sector[bytes];

        while (cluster < 0x0FFFFFF8)
        {
                DWORD first_root = FatFirstSectorForCluster(vol, cluster);
                for (size_t clusterOffset = 0; clusterOffset < bt->bpb.SectorsPerCluster; ++clusterOffset)
                {
                        ReadFrom(FatSectorToOffset(vol, first_root + clusterOffset), sector, bytes, vol->Drive);
                        memcpy(Directory, sector, sizeof(Directory));

                        for (size_t i = 0; i < entries; ++i)
                        {
                                if (Directory[i].Name[0] == 0x00)
                                        goto done;
                                if (Directory[i].Name[0] == FAT_UNUSED)
                                        continue;
                                if (Directory[i].Flags == FAT_LONG_FILE_NAME)
                                {
                                        SerialPrint(" [WARN] FAT Subsystem: Long File Names are not supported, Ignoring\r\n");
                                        continue;
                                }

                                if (!strncmp((char *)Directory[i].Name, (char *)Name, 8) &&
                                    !strncmp((char *)Directory[i].Ext,  (char *)Ext,  3))
                                {
                                        Result.Cluster = cluster;
                                        Result.Found   = 1;
                                        Result.Index   = i;
                                        Result.Offset  = clusterOffset;
                                        Result.Dir     = Directory[i];
                                        return Result;
                                }
                        }
                }
                cluster = FatNextCluster(vol, cluster);
        }
done:
        return Result;
}

DWORD FatFindFreeCluster(FATVolume *vol)
{
        FATBootSector *bt = &vol->Boot;
        DWORD totalClusters = FatClusterCount(vol);
        DWORD fatStart = FatFirstFat(vol);

        for (DWORD i = 2; i < totalClusters + 2; i++)
        {
                DWORD fatOffset = i * 4;
                DWORD fatSector = fatStart + (fatOffset / bt->bpb.BytesPerSector);
                DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
                BYTE  sector[bt->bpb.BytesPerSector];
                ReadFrom(FatSectorToOffset(vol, fatSector), sector, bt->bpb.BytesPerSector, vol->Drive);
                DWORD entry = *(DWORD *)&sector[entOffset] & 0x0FFFFFFF;
                if (entry == 0)
                        return i;
        }
        return 0;
}

void FatSetCluster(FATVolume *vol, DWORD cluster, DWORD value)
{
        FATBootSector *bt = &vol->Boot;
        DWORD bytes     = bt->bpb.BytesPerSector;
        DWORD fatOffset = cluster * 4;
        DWORD fatSector = FatFirstFat(vol) + (fatOffset / bytes);
        DWORD entOffset = fatOffset % bytes;
        BYTE  sector[bytes];
        ReadFrom(FatSectorToOffset(vol, fatSector), sector, bytes, vol->Drive);
        DWORD *entry = (DWORD *)&sector[entOffset];
        *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);
        WriteTo(FatSectorToOffset(vol, fatSector), sector, bytes, vol->Drive);
        WriteTo(FatSectorToOffset(vol, fatSector + FatSize(vol)), sector, bytes, vol->Drive);
}

DWORD FatAllocateCluster(FATVolume *vol)
{
        DWORD freeCluster = FatFindFreeCluster(vol);
        if (freeCluster == 0)
                return 0;
        FatSetCluster(vol, freeCluster, 0x0FFFFFF8);
        return freeCluster;
}

DWORD FatWriteData(FATVolume *vol, BYTE *data, DWORD size)
{
        FATBootSector *bt = &vol->Boot;
        DWORD firstCluster = 0;
        DWORD prevCluster  = 0;
        DWORD remaining    = size;
        BYTE *ptr          = data;

        DWORD sectorsPerCluster = bt->bpb.SectorsPerCluster;
        DWORD bytesPerCluster   = sectorsPerCluster * bt->bpb.BytesPerSector;

        while (remaining > 0)
        {
                DWORD cluster = FatAllocateCluster(vol);
                if (cluster == 0)
                        return 0;

                if (firstCluster == 0) firstCluster = cluster;
                if (prevCluster  != 0) FatSetCluster(vol, prevCluster, cluster);

                DWORD firstSector = FatFirstSectorForCluster(vol, cluster);
                DWORD writeSize   = remaining < bytesPerCluster ? remaining : bytesPerCluster;

                for (DWORD i = 0; i < sectorsPerCluster && writeSize > 0; i++)
                {
                        DWORD sectorSize = writeSize < bt->bpb.BytesPerSector ? writeSize : bt->bpb.BytesPerSector;
                        WriteTo(FatSectorToOffset(vol, firstSector + i), ptr, bt->bpb.BytesPerSector, vol->Drive);
                        ptr       += sectorSize;
                        writeSize -= sectorSize;
                        remaining -= sectorSize;
                }

                prevCluster = cluster;
        }

        return firstCluster;
}

DWORD FatFindFreeEntry(FATVolume *vol, DWORD dirCluster)
{
        FATBootSector *bt = &vol->Boot;
        DWORD cluster = dirCluster;
        size_t entriesPerSector = bt->bpb.BytesPerSector / sizeof(FATDirectory);

        while (cluster < 0x0FFFFFF8)
        {
                DWORD firstSector = FatFirstSectorForCluster(vol, cluster);

                for (DWORD s = 0; s < bt->bpb.SectorsPerCluster; s++)
                {
                        BYTE sector[bt->bpb.BytesPerSector];
                        ReadFrom(FatSectorToOffset(vol, firstSector + s), sector, bt->bpb.BytesPerSector, vol->Drive);
                        FATDirectory *dir = (FATDirectory *)sector;
                        for (size_t i = 0; i < entriesPerSector; i++)
                        {
                                if (dir[i].Name[0] == 0x00 || dir[i].Name[0] == 0xE5)
                                        return (cluster << 16) | (s << 8) | i;
                        }
                }

                cluster = FatNextCluster(vol, cluster);
        }

        return 0;
}

void FatWriteDirectoryEntry(FATVolume *vol, DWORD location, FATDirectory *entry)
{
        FATBootSector *bt = &vol->Boot;
        DWORD cluster     = location >> 16;
        DWORD s           = (location >> 8) & 0xFF;
        DWORD index       = location & 0xFF;
        DWORD firstSector = FatFirstSectorForCluster(vol, cluster);
        BYTE  sector[bt->bpb.BytesPerSector];
        ReadFrom(FatSectorToOffset(vol, firstSector + s), sector, bt->bpb.BytesPerSector, vol->Drive);
        FATDirectory *dir = (FATDirectory *)sector;
        memcpy(&dir[index], entry, sizeof(FATDirectory));
        WriteTo(FatSectorToOffset(vol, firstSector + s), sector, bt->bpb.BytesPerSector, vol->Drive);
}

FATDirectory FatCreateEntry(const char *name, DWORD firstCluster, DWORD size, BYTE attributes)
{
        FATDirectory entry = {0};
        char name83[8], ext[3];
        FatConvert83(name, name83, ext);
        memcpy(entry.Name, name83, 8);
        memcpy(entry.Ext,  ext,    3);
        entry.Flags               = attributes;
        entry.EntryFirstClusterLow  = firstCluster & 0xFFFF;
        entry.EntryFirstClusterHigh = (firstCluster >> 16) & 0xFFFF;
        entry.Size                = size;
        return entry;
}

int FatWrite(const char *path, BYTE *data, DWORD size, FATVolume *vol, FATDirectory *Parent)
{
        DWORD parentCluster = Parent ? (Parent->EntryFirstClusterHigh << 16 | Parent->EntryFirstClusterLow) : FatRootCluster(vol);
        DWORD firstCluster  = FatWriteData(vol, data, size);
        if (firstCluster == 0)
                return -1;

        FATDirectory entry = FatCreateEntry(path, firstCluster, size, FAT_ARCHIVE);
        DWORD location     = FatFindFreeEntry(vol, parentCluster);
        if (location == 0)
                return -2;

        FatWriteDirectoryEntry(vol, location, &entry);
        return 0;
}

void *FatRead(BYTE Name[8], BYTE Ext[3], FATVolume *vol, FATDirectory *Parent)
{
        if (Parent && !(Parent->Flags & FAT_DIRECTORY))
                return NULL;

        FATBootSector *bt = &vol->Boot;
        const size_t bytes = bt->bpb.BytesPerSector;
        FATFileLocation Location = FatLocateInDir(Name, Ext, vol, Parent);

        if (!Location.Found)
                return NULL;

        BYTE *Data = kalloc(Location.Dir.Size + 1);
        BYTE *p    = Data;
        memset(Data, 0, Location.Dir.Size + 1);

        DWORD cluster   = Location.Dir.EntryFirstClusterHigh << 16 | Location.Dir.EntryFirstClusterLow;
        DWORD remaining = Location.Dir.Size;
        BYTE  Sector[bytes];

        while (cluster < 0x0FFFFFF8)
        {
                DWORD sector = FatFirstSectorForCluster(vol, cluster);
                for (size_t off = 0; off < bt->bpb.SectorsPerCluster && remaining > 0; ++off)
                {
                        ReadFrom(FatSectorToOffset(vol, sector + off), Sector, bytes, vol->Drive);
                        DWORD chunk = remaining < bytes ? remaining : bytes;
                        memcpy(p, Sector, chunk);
                        p         += chunk;
                        remaining -= chunk;
                }
                cluster = FatNextCluster(vol, cluster);
        }

        return Data;
}

void FatConvert83(const char *path, char *NameOut, char *ExtOut)
{
        memset(NameOut, ' ', 8);
        memset(ExtOut,  ' ', 3);

        size_t n = 0;
        while (*path && *path != '.' && n < 8)
                NameOut[n++] = *path++;

        if (*path == '.') path++;

        size_t e = 0;
        while (*path && e < 3 && *path != '/')
                ExtOut[e++] = *path++;
}

int Init(void)
{
        VNode *Root = RootVNode();
        VNode *Mnt = Root->RelativeFind(Root, "/mnt", 4);
        VNode *dev_dir = Root->RelativeFind(Root, "/dev", 4);
        VNode *Node = dev_dir->FirstChild;
        FATBootSector bs = {0};
        (void)Mnt;

        while (Node)
        {
                if (Node->Name.Length == 4 &&
                    Node->Name.Name[0] == 't' &&
                    Node->Name.Name[1] == 't' &&
                    Node->Name.Name[2] == 'y' &&
                    Node->Name.Name[3] == '0')
                {
                        Node = Node->Next;
                        continue;
                }

                if (Node->ReadFunction)
                {
                        int result = Node->ReadFunction(&bs, sizeof(bs), 1, Node);
                        if (result == 1 && bs.Magic == FAT_BOOT_SECTOR_SIGN)
                        {
                                SerialPrint(" [FAT] Found potential FAT device: %s\r\n", Node->Name.Name);
                                FATVolume vol = FatMount(Node);
                                SerialPrint("\r\n-- FAT-XX TEST FOR DRIVE @%p --\r\n", Node);
                                SerialPrint("        fat.type                %d\r\n", FatIdentify(&vol));
                                SerialPrint("        fat.total-sect          %d\r\n", FatTotalSectors(&vol));
                                SerialPrint("        fat.size                %d\r\n", FatSize(&vol));
                                SerialPrint("        fat.root-size           %d\r\n", FatRootDirSize(&vol));
                                SerialPrint("        fat.first-dat           %d\r\n", FatFirstData(&vol));
                                SerialPrint("        fat.first-fat           %d\r\n", FatFirstFat(&vol));
                                SerialPrint("        fat.data-cnt            %d\r\n", FatDataCount(&vol));
                                SerialPrint("        fat.cluster-cnt         %d\r\n", FatClusterCount(&vol));
                                SerialPrint("        fat.first-root          %d\r\n", FatFirstRoot(&vol));
                                SerialPrint("        fat.root-clust          %d\r\n", FatRootCluster(&vol));
                                SerialPrint("        fat.ssize  %d\r\n", vol.Boot.bpb.BytesPerSector);
                        }
                }

                Node = Node->Next;
        }

        return 0;
}
