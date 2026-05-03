#include <fat/main.h>
#include <vmem/alloc.h>
#include <string.h>

DWORD FatTotalSectors(FATBootSector *Bt)
{
        return (Bt->bpb.TotalSectorsInVolume) ? Bt->bpb.TotalSectorsInVolume : Bt->bpb.ExtendedSectorCount;
}

DWORD FatSize(FATBootSector *Bt)
{
        return (Bt->bpb.SectorsPerFAT) ? Bt->bpb.SectorsPerFAT : Bt->as.fat32.SectorsPerFAT;
}

/* If FAT32 then ignore // will be 0 */
DWORD FatRootDirSize(FATBootSector *Bt)
{
        return ((Bt->bpb.RootDirectoryEntries * sizeof(FATDirectory)) + (Bt->bpb.BytesPerSector - 1)) / Bt->bpb.BytesPerSector;
}

DWORD FatFirstData(FATBootSector *Bt)
{
        return (Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(Bt)) + FatRootDirSize(Bt));
}

DWORD FatFirstFat(FATBootSector *Bt)
{
        return (Bt->bpb.ReservedSectors);
}

DWORD FatDataCount(FATBootSector *Bt)
{
        return FatTotalSectors(Bt) - (Bt->bpb.ReservedSectors + (Bt->bpb.FATCount * FatSize(Bt)) + FatRootDirSize(Bt));
}

DWORD FatClusterCount(FATBootSector *Bt)
{
        return FatDataCount(Bt) / Bt->bpb.SectorsPerCluster;
}

FATType FatIdentify(FATBootSector *Bt)
{
        DWORD TotalClusters = FatClusterCount(Bt);
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

DWORD FatFirstRoot(FATBootSector *Bt)
{
        return FatFirstData(Bt) - FatRootDirSize(Bt);
}

DWORD FatRootCluster(FATBootSector *Bt)
{
        return Bt->as.fat32.RootCluster;
}

DWORD FatFirstSectorForCluster(FATBootSector *Bt, DWORD Cluster)
{
        return ((Cluster - 2) * Bt->bpb.SectorsPerCluster) + FatFirstData(Bt);
}

void FatConvertPaddedToNull(char *X, size_t L)
{
        for (size_t i = 0; i < L; ++i)
                X[i] = (X[i] == ' ') ? '\0' : X[i];
}

DWORD FatNextCluster32(FATBootSector *bt, DWORD cluster, VNode *Device)
{
        DWORD fatOffset = cluster * 4;
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        Device->FileOffset = fatSector * bt->bpb.BytesPerSector;
        Device->ReadFunction(sector, bt->bpb.BytesPerSector, 1, Device);
        DWORD entry = *(DWORD *)&sector[entOffset];
        return entry & 0x0FFFFFFF;
}

WORD FatNextCluster16(FATBootSector *bt, WORD cluster, VNode *Device)
{
        DWORD fatOffset = cluster * 2;
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        Device->FileOffset = fatSector * bt->bpb.BytesPerSector;
        Device->ReadFunction(sector, bt->bpb.BytesPerSector, 1, Device);
        return *(WORD *)&sector[entOffset];
}

WORD FatNextCluster12(FATBootSector *bt, WORD cluster, VNode *Device)
{
        DWORD fatOffset = cluster + (cluster / 2);
        DWORD fatSector = FatFirstFat(bt) + (fatOffset / bt->bpb.BytesPerSector);
        DWORD entOffset = fatOffset % bt->bpb.BytesPerSector;
        BYTE  sector[bt->bpb.BytesPerSector];
        Device->FileOffset = fatSector * bt->bpb.BytesPerSector;
        Device->ReadFunction(sector, bt->bpb.BytesPerSector, 1, Device);
        DWORD entry = *(DWORD *)&sector[entOffset];
        if (cluster & 1)
                entry >>= 4;
        else
                entry &= 0x0FFF;
        return entry;
}

DWORD FatNextCluster(FATBootSector *bt, DWORD cluster, VNode *Drive)
{
        switch (FatIdentify(bt))
        {
        case FAT_UNKNOWN:
                return -1;
        case FAT_12:
                return FatNextCluster12(bt, cluster, Drive);
        case FAT_16:
                return FatNextCluster16(bt, cluster, Drive);
        case FAT_32:
                return FatNextCluster32(bt, cluster, Drive);
        }
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
                                SerialPrint(" [FAT] Found potential FAT device: ");
                                SerialPrint(Node->Name.Name);
                                SerialPrint("\r\n");
                        }
                }

                Node = Node->Next;
        }

        return 0;
}
