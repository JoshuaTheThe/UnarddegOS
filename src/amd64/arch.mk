override ARCH := amd64
override ARCH_CFLAGS += -m64 -fno-stack-protector -nostdlib -ffreestanding -fno-pie
override ARCH_ASFLAGS += -m64 -nostdlib
override ARCH_KLDFLAGS += -melf_x86_64
override ARCH_OUTPUT_SUFFIX := _amd64.o
override ARCH_LINKER_SCRIPT := src/amd64/linker.ld
override ARCH_RUN_SCRIPT := src/amd64/run.sh

override KCC = clang

MODULE_NAMES += pci fat
