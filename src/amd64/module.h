
#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stddef.h>

struct elf_header
{
        unsigned char magic[4];
        unsigned char bits; // 2 = 64-bit
        unsigned char endian;
        unsigned char version;
        unsigned char os_abi;
        unsigned char abi_version;
        unsigned char padding[7];
        uint16_t type;
        uint16_t machine; // 0x3E = AMD64
        uint32_t version2;
        uint64_t entry; // 64-bit
        uint64_t phoff;
        uint64_t shoff;
        uint32_t flags;
        uint16_t ehsize;
        uint16_t phentsize;
        uint16_t phnum;
        uint16_t shentsize;
        uint16_t shnum;
        uint16_t shstrndx;
};

struct elf_section_header
{
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
};

struct elf_symbol
{
        uint32_t st_name;
        unsigned char st_info;
        unsigned char st_other;
        uint16_t st_shndx;
        uint64_t st_value;
        uint64_t st_size;
};

struct elf_rela
{
        uint64_t r_offset; // offset to patch
        uint64_t r_info;   // symbol + type
        int64_t r_addend;  // constant addend
};

struct elf_rel
{
        uint64_t r_offset;
        uint64_t r_info;
};

/* ---------- Relocation type macros ---------- */
#define ELF64_R_TYPE(info) ((info) & 0xFFFFFFFF)
#define ELF64_R_SYM(info) ((info) >> 32)

/* ---------- Relocation types (AMD64) ---------- */
#define R_X86_64_NONE 0
#define R_X86_64_64 1   // S + A
#define R_X86_64_PC32 2 // S + A - P
#define R_X86_64_GOT32 3
#define R_X86_64_PLT32 4
#define R_X86_64_COPY 5
#define R_X86_64_GLOB_DAT 6  // S
#define R_X86_64_JUMP_SLOT 7 // S
#define R_X86_64_RELATIVE 8  // B + A
#define R_X86_64_GOTPCREL 9
#define R_X86_64_32 10
#define R_X86_64_32S 11
#define R_X86_64_16 12
#define R_X86_64_PC16 13
#define R_X86_64_PC8 14
#define R_X86_64_PC64 24 // S + A - P (64-bit)

/* ---------- Section types (unchanged) ---------- */
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_INIT_ARRAY 14
#define SHT_FINI_ARRAY 15
#define SHT_PREINIT_ARRAY 16
#define SHT_GROUP 17
#define SHT_SYMTAB_SHNDX 18

/* Section flags */
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
// ... rest unchanged, omitted for brevity

/* Special section indices (same as before) */
#define SHN_UNDEF 0
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2

/* File types */
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

struct loaded_module
{
        void *base;
        size_t size;
        const char *name;
        struct elf_symbol *symtab;
        const char *strtab;
        uint32_t symcount;
};

void LoadModule(void *addr, size_t size, const char *name);
void *FindKernelSymbol(const char *name);

#endif
