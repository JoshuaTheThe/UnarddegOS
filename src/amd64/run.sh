#!/bin/bash
clear
mkdir -p bin/isodir

if grub-file --is-x86-multiboot2 bin/unarddegos_amd64.o; then
    echo "the file is multiboot2"
    mkdir -p bin/isodir/boot/grub
    cp bin/unarddegos_amd64.o bin/isodir/boot
    cp -rf bin/modules bin/isodir/boot
    cp src/amd64/grub.cfg bin/isodir/boot/grub/grub.cfg
    dd if=/dev/zero of=bin/unarddegos_amd64.hdd bs=1M count=64
    parted -s bin/unarddegos_amd64.hdd mklabel msdos
    parted -s bin/unarddegos_amd64.hdd mkpart primary fat32 1MiB 100%
    LOOP=$(sudo losetup --show -f --partscan bin/unarddegos_amd64.hdd)
    echo "Loop device: ${LOOP}"
    echo "Partition: ${LOOP}p1"
    sudo mkfs.fat -F32 "${LOOP}p1"
    sudo mount "${LOOP}p1" /mnt
    sudo grub-install --target=i386-pc --root-directory=/mnt --boot-directory=/mnt/boot ${LOOP}
    sudo mkdir -p /mnt/boot/grub
    sudo cp bin/unarddegos_amd64.o /mnt/boot/
    sudo cp -rf bin/modules /mnt/boot/
    sudo cp src/amd64/grub.cfg /mnt/boot/grub/
    sudo umount /mnt
    sudo losetup -d ${LOOP}
    qemu-system-x86_64 -hda bin/unarddegos_amd64.hdd -m 256 -cpu max -nographic \
        -d cpu_reset,guest_errors -D qemu.log
else
    echo "the file is not multiboot2"
fi
