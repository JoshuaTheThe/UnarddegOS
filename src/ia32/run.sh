#!/bin/bash
clear
mkdir -p bin/isodir

if grub-file --is-x86-multiboot2 bin/unarddegos_ia32.o; then
    echo "the file is multiboot"
    mkdir -p bin/isodir/boot/grub
    cp bin/unarddegos_ia32.o bin/isodir/boot
    cp -rf bin/modules bin/isodir/boot
    cp src/ia32/grub.cfg bin/isodir/boot/grub/grub.cfg
    
    # Create HDD image instead of ISO
    dd if=/dev/zero of=bin/unarddegos_ia32.hdd bs=1M count=64
    
    # Partition and format (using parted and mkfs)
    parted -s bin/unarddegos_ia32.hdd mklabel msdos
    parted -s bin/unarddegos_ia32.hdd mkpart primary fat32 1MiB 100%
    
    # Setup loop device WITH PARTITION SCANNING
    LOOP=$(sudo losetup --show -f --partscan bin/unarddegos_ia32.hdd)
    echo "Loop device: ${LOOP}"
    echo "Partition: ${LOOP}p1"
    
    # Format the partition
    sudo mkfs.fat -F32 "${LOOP}p1"
    
    # Mount
    sudo mount "${LOOP}p1" /mnt
    
    # Install GRUB to HDD
    sudo grub-install --target=i386-pc --root-directory=/mnt --boot-directory=/mnt/boot ${LOOP}
    
    # Copy kernel and modules
    sudo mkdir -p /mnt/boot/grub
    sudo cp bin/unarddegos_ia32.o /mnt/boot/
    sudo cp -rf bin/modules /mnt/boot/
    sudo cp src/ia32/grub.cfg /mnt/boot/grub/
    
    # Cleanup
    sudo umount /mnt
    sudo losetup -d ${LOOP}
    
    # Run QEMU with HDD
    qemu-system-x86_64 -debugcon stdio -hda bin/unarddegos_ia32.hdd -m 64
else
    echo "the file is not multiboot"
fi
