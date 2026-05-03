
#include <vmem/alloc.h>
#include <panic.h>

void *kalloc(size_t Size)
{
        if (Size == 0)
                return NULL;
        size_t TotalSize = Size + sizeof(BlockHeader);
        size_t PagesNeeded = (TotalSize + PAGE_SIZE - 1) / PAGE_SIZE;
        void *Base = AllocatePage(); // warning - currently unsafe due to memory fragmentation, will be replaced with virtual handler in future
        if (!Base)
        {
                Panic(PANIC_RAN_OUT_OF_MEMORY);
        }

        BlockHeader *Header = (BlockHeader *)Base;
        Header->Size = TotalSize;
        Header->Next = NULL;
        for (size_t i = 1; i < PagesNeeded; ++i)
        {
                void *NextPage = AllocatePage();
                if (!NextPage)
                {
                        for (size_t j = 0; j < i; ++j)
                        {
                                FreePage((char *)Base + j * PAGE_SIZE);
                        }
                        Panic(PANIC_RAN_OUT_OF_MEMORY);
                }
        }

        return (char *)Base + sizeof(BlockHeader);
}

void kfree(void *Base)
{
        PanicIfNull(Base);
        BlockHeader *Header = (BlockHeader *)((char *)Base - sizeof(BlockHeader));
        size_t TotalSize = Header->Size;
        size_t PagesUsed = (TotalSize + PAGE_SIZE - 1) / PAGE_SIZE;
        void *StartPage = (char *)Base - sizeof(BlockHeader);
        for (size_t i = 0; i < PagesUsed; ++i)
        {
                FreePage((char *)StartPage + i * PAGE_SIZE);
        }
}
