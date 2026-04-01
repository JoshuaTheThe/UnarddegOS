
#include <vmem/bumpalloc.h>
#include <string.h>
#include <panic.h>

uint8_t memory_pool[MEMORY_SIZE];
size_t current_index = 0;

void *BumpAllocate(const size_t size)
{
        const size_t aligned_size = (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
        if (current_index + aligned_size > MEMORY_SIZE)
        {
                Panic(PANIC_RAN_OUT_OF_MEMORY);
                return NULL;
        }
        void *const ptr = &memory_pool[current_index];
        current_index += aligned_size;
        Trace(memset(ptr, 0, size));
        return ptr;
}
