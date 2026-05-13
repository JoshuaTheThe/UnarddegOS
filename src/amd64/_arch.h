#ifndef _ARCH_H
#define _ARCH_H

#define HAS_TEMPERATURE
#define MODULE
#include <stdint.h>
#pragma pack(push)
#pragma pack(1)

struct multiboot_tag
{
        uint32_t type;
        uint32_t size;
} __attribute__((packed));

struct multiboot_info
{
        uint32_t total_size;
        uint32_t reserved;
        struct multiboot_tag tags[];
} __attribute__((packed));

struct multiboot_tag_module
{
        uint32_t type;
        uint32_t size;
        uint32_t mod_start;
        uint32_t mod_end;
        char cmdline[];
} __attribute__((packed));

#pragma pack(pop)

#endif
