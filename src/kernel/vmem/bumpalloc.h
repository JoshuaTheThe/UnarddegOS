
#ifndef BUMPALLOC_H
#define BUMPALLOC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MEMORY_SIZE (1024 * 128)
#define ALIGNMENT 8

void *BumpAllocate(size_t size);

#endif
