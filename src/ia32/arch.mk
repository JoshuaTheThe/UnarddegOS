override ARCH := ia32
override ARCH_CFLAGS += -m32 -fno-stack-protector -nostdlib -ffreestanding  -fpack-struct
override ARCH_ASFLAGS += -m32 -fno-stack-protector -nostdlib -ffreestanding
override ARCH_KLDFLAGS += -melf_i386
override ARCH_OUTPUT_SUFFIX := _ia32.o
override ARCH_LINKER_SCRIPT := src/ia32/linker.ld
override ARCH_RUN_SCRIPT := src/ia32/run.sh
override ARCH_MODULE_FLAGS := -mno-tls-direct-seg-refs -fno-tls-model -U__TLS__  -fpack-struct
override KCC = clang

MODULE_NAMES += pci ide fat shell hello ps2 vga
