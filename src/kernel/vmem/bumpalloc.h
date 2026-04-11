
#ifndef BUMPALLOC_H
#define BUMPALLOC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MEMORY_SIZE (1024 * 64)
#define ALIGNMENT 4

void *BumpAllocate(size_t size);

#endif
