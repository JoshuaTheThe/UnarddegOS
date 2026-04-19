
MODULES_DIR := src/modules
MODULES_BUILD_DIR := obj/modules
MODULES_BIN_DIR := bin/modules

MODULES := $(wildcard $(MODULES_DIR)/*/)
# MODULE_NAMES := $(notdir $(patsubst %/,%,$(MODULES)))
MODULE_NAMES += hello

MODULE_OBJS := $(foreach m,$(MODULE_NAMES),$(MODULES_BUILD_DIR)/$(m)/main.o)
MODULE_BINS := $(foreach m,$(MODULE_NAMES),$(MODULES_BIN_DIR)/$(m).ko)

.PHONY: modules clean-modules

modules: $(MODULE_BINS)

$(MODULES_BIN_DIR)/%.ko: $(MODULES_BUILD_DIR)/%/main.o
	@mkdir -p $(dir $@)
	$(KLD) $(KLDFLAGS) -r -o $@ $< 

$(MODULES_BUILD_DIR)/%/main.o: $(MODULES_DIR)/%/main.c
	@mkdir -p $(dir $@)
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -fno-pic -fno-pie -I src/kernel -I src/modules -I $(SRC)/$(ARCH) -c -o $@ $< -mno-tls-direct-seg-refs -fno-tls-model -U__TLS__

clean-modules:
	rm -rf $(MODULES_BUILD_DIR) $(MODULES_BIN_DIR)
