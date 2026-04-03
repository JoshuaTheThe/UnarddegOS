#!/bin/bash
clear
mkdir bin/isodir
if grub-file --is-x86-multiboot bin/unarddegos_ia32.o; then
        echo the file is multiboot
        mkdir -p bin/isodir/boot/grub
        cp bin/unarddegos_ia32.o bin/isodir/boot
        cp src/ia32/grub.cfg bin/isodir/boot/grub/grub.cfg
        grub-mkrescue -o bin/unarddegos_ia32.iso bin/isodir
        qemu-system-x86_64 -debugcon stdio -cdrom bin/unarddegos_ia32.iso -m 64 -boot d
else
        echo the file is not multiboot
fi
