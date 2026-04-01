#!/bin/bash
clear
qemu-system-arm -M vexpress-a15 -cpu cortex-a15 \
    -device loader,file=bin/unarddegos_arm32.o,cpu-num=0 \
    -S