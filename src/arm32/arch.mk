override ARCH := arm32
override ARCH_CFLAGS += -marm -mfpu=vfpv3-d16 -mfloat-abi=hard
override ARCH_ASFLAGS += -march=armv7-a -mcpu=cortex-a15
override ARCH_KLDFLAGS += -marmelf
override ARCH_OUTPUT_SUFFIX := _arm32.o
override ARCH_LINKER_SCRIPT := src/arm32/linker.ld
override ARCH_RUN_SCRIPT := src/arm32/run.sh

override KCC = clang
override KLD = arm-none-eabi-ld
override KCFLAGS += -target armv7-none-eabi -nostdlib -m32
override KAS = arm-none-eabi-as
override ASFLAGS =
