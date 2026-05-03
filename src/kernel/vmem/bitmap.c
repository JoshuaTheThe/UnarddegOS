
#include <vmem/bitmap.h>
#include <string.h>
#include <panic.h>

static uint8_t   PageBitmap[TOTAL_BITMAP / 8] = {0};
static uintptr_t MemoryStart = 0x400000;

size_t MStat(void)
{
        size_t Free = 0;
        for (size_t i = 0; i < TOTAL_BITMAP; i++)
        {
                size_t ByteIdx = i / 8;
                size_t BitIdx = i % 8;
                if (!(PageBitmap[ByteIdx] & (1 << BitIdx)))
                {
                        Free += 1;
                }
        }

        return Free;
}

int GetFreePage(void)
{
        for (size_t i = 0; i < TOTAL_BITMAP; i++)
        {
                size_t ByteIdx = i / 8;
                size_t BitIdx = i % 8;
                if (!(PageBitmap[ByteIdx] & (1 << BitIdx)))
                {
                        PageBitmap[ByteIdx] |= (1 << BitIdx);
                        return i;
                }
        }
        return -1;
}

void FreePage(void *Page)
{
        uintptr_t Addr = (uintptr_t)Page;
        size_t PageIdx = (Addr - MemoryStart) / PAGE_SIZE;
        size_t ByteIdx = PageIdx / 8;
        size_t BitIdx = PageIdx % 8;
        PageBitmap[ByteIdx] &= ~(1 << BitIdx);
}

void *AllocatePage(void)
{
        int PageIdx = GetFreePage();
        if (PageIdx == -1)
                Panic(PANIC_RAN_OUT_OF_MEMORY);
        void *Page = (void*)(MemoryStart + (PageIdx * PAGE_SIZE));
        memset(Page, 0, PAGE_SIZE);
        return Page;
}

