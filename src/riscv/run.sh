#!/bin/bash
clear
qemu-system-riscv64 \
    -machine virt -bios none -kernel bin/unarddegos_riscv.o