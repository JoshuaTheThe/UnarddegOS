override ARCH := riscv
override ARCH_CFLAGS += -mcmodel=medany
override ARCH_OUTPUT_SUFFIX := _riscv.o
override ARCH_LINKER_SCRIPT := src/riscv/linker.ld
override ARCH_RUN_SCRIPT := src/riscv/run.sh

override KCC = riscv64-unknown-elf-gcc
override KAS = riscv64-unknown-elf-as
override KLD = riscv64-unknown-elf-ld
