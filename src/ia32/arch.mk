override ARCH := ia32
override ARCH_CFLAGS += -m32 -fno-stack-protector -nostdlib -ffreestanding
override ARCH_ASFLAGS += -m32 -fno-stack-protector -nostdlib -ffreestanding
override ARCH_KLDFLAGS += -melf_i386
override ARCH_OUTPUT_SUFFIX := _ia32.o
override ARCH_LINKER_SCRIPT := src/ia32/linker.ld
override ARCH_RUN_SCRIPT := src/ia32/run.sh

override KCC = clang

MODULE_NAMES += pci ide fat
