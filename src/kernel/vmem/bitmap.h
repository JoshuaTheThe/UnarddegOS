
#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>

#define TOTAL_BITMAP (1024 * 1024)
#define PAGE_SIZE (4096)

void InitPageAllocator(uintptr_t StartAddress, size_t MemorySize);
int GetFreePage(void);
void FreePage(void *Page);
void *AllocatePage(void);
size_t MStat(void);

#endif

