
#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stddef.h>

struct elf_header
{
        unsigned char magic[4];
        unsigned char bits;
        unsigned char endian;
        unsigned char version;
        unsigned char os_abi;
        unsigned char abi_version;
        unsigned char padding[7];
        unsigned short type;
        unsigned short machine;
        unsigned int version2;
        unsigned int entry;
        unsigned int phoff;
        unsigned int shoff;
        unsigned int flags;
        unsigned short ehsize;
        unsigned short phentsize;
        unsigned short phnum;
        unsigned short shentsize;
        unsigned short shnum;
        unsigned short shstrndx;
};

struct elf_section_header
{
        unsigned int sh_name;
        unsigned int sh_type;
        unsigned int sh_flags;
        unsigned int sh_addr;
        unsigned int sh_offset;
        unsigned int sh_size;
        unsigned int sh_link;
        unsigned int sh_info;
        unsigned int sh_addralign;
        unsigned int sh_entsize;
};

struct elf_symbol
{
        unsigned int st_name;
        unsigned int st_value;
        unsigned int st_size;
        unsigned char st_info;
        unsigned char st_other;
        unsigned short st_shndx;
};

struct elf_rela
{
        unsigned int r_offset;
        unsigned int r_info;
        int r_addend;
};

#define ELF32_R_TYPE(info) ((info) & 0xFF)
#define ELF32_R_SYM(info) ((info) >> 8)

#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2
#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JUMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10

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

// ELF section flags
#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 0xf0000000

// Special section indices
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

// ELF file types
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

// i386 relocation types
#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2
#define R_386_GOT32 3
#define R_386_PLT32 4
#define R_386_COPY 5
#define R_386_GLOB_DAT 6
#define R_386_JUMP_SLOT 7
#define R_386_RELATIVE 8
#define R_386_GOTOFF 9
#define R_386_GOTPC 10
#define R_386_32PLT 11

// ELF structures (add to module.h if not there)
struct elf_rel
{
        uint32_t r_offset;
        uint32_t r_info;
};

typedef int (*module_init_t)(void);

struct loaded_module
{
        void *base;
        size_t size;
        const char *name;
        struct elf_symbol *symtab;
        const char *strtab;
        int symcount;
};

void LoadModule(void *addr, size_t size, const char *name);
void *FindKernelSymbol(const char *name);
void LoadModules(unsigned int magic, unsigned int mb_info_addr);

#endif
