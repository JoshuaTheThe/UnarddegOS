
.PHONY: arch
arch:
	arm-none-eabi-objcopy -Obinary bin/$(OUTPUT) bin/$(OUTPUT).bin
	hexdump -C bin/$(OUTPUT).bin | head