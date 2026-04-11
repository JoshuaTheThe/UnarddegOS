
override MAKEFLAGS += -rR

ARCH := riscv
override BIN := bin
override SRC := src
override KERNEL := $(SRC)/kernel
override OUTPUT := unarddegos

override CFILES := $(shell find $(KERNEL)/ -type f -name '*.c' | sed 's|^$(SRC)/||' | LC_ALL=C sort)
override CFILES += $(shell find $(SRC)/$(ARCH) -type f -name '*.c' | sed 's|^$(SRC)/||' | LC_ALL=C sort)
override ASFILES := $(shell find $(KERNEL)/ -type f -name '*.s' | sed 's|^$(SRC)/||' | LC_ALL=C sort)
override ASFILES += $(shell find $(SRC)/$(ARCH) -type f -name '*.s' | sed 's|^$(SRC)/||' | LC_ALL=C sort)

override KCC := clang
override KAS := clang
override KLD := ld
override KCFLAGS := -pipe -Wall -Wextra -c -g -fno-strict-aliasing -O0
override KCFLAGS += 
override ASFLAGS += 
override KLDFLAGS :=

override ARCH_CFLAGS :=
override ARCH_ASFLAGS :=
override ARCH_KLDFLAGS :=
override ARCH_LINKER_SCRIPT := $(SRC)/$(ARCH)/linker.ld
override ARCH_OUTPUT_SUFFIX :=
override ARCH_RUN_SCRIPT :=

include $(SRC)/$(ARCH)/arch.mk

override KCFLAGS += \
    -c \
    -std=gnu11 \
    -ffreestanding \
    -fno-builtin \
    -Werror \
    -fno-stack-protector \
    -fno-stack-check \
    $(ARCH_CFLAGS)

override ASFLAGS += \
    -c \
    $(ARCH_ASFLAGS)

override KLDFLAGS += \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    -T $(ARCH_LINKER_SCRIPT) \
    $(ARCH_KLDFLAGS)

override OUTPUT := $(OUTPUT)$(ARCH_OUTPUT_SUFFIX)
override COBJ := $(addprefix obj/,$(CFILES:.c=.c.o))
override ASOBJ := $(addprefix obj/,$(ASFILES:.s=.s.o))

OBJ := $(ASOBJ) $(COBJ)

override HEADER_DEPS := $(addprefix obj/,$(CFILES:.c=.c.d))
override HEADER_DEPS += $(addprefix obj/,$(CFILES:.C=.C.d))

.PHONY: all
all: bin/$(OUTPUT)

bin/$(OUTPUT): $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@
	@echo " [INFO] Built $(OUTPUT) for architecture $(ARCH)"

-include $(HEADER_DEPS)

obj/%.c.o: $(SRC)/%.c
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@ -I $(KERNEL) -I $(SRC)/$(ARCH)

obj/%.s.o: $(SRC)/%.s
	mkdir -p "$$(dirname $@)"
	$(KAS) $(ASFLAGS) -c $< -o $@

obj/%.s.o: $(SRC)/%.s
	mkdir -p "$$(dirname $@)"
	$(KAS) $(ASFLAGS) -c $< -o $@

include $(SRC)/$(ARCH)/post.mk

.PHONY: clean
clean:
	@rm -rf bin obj

.PHONY: run
run:
	@bash $(ARCH_RUN_SCRIPT)
