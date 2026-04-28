# OS/11 - A Modular Unix-like Kernel
Kernel made for educational purposes.

**Version:** 0.0.80 (major.minor.commit)
**Architectures:** x86 (ia32), ARMv7, RISC-V 64  
**License:** idgaf, MIT

## Overview

OS/11 is a microkernel-style, Unix-like operating system written from scratch. It features a VFS-centric design where **everything is a file** - processes, devices, PCI buses, and kernel internals all appear in the VFS tree. The kernel itself knows nothing about filesystems, drivers, or devices - it only understands VNodes.

### Philosophy

- **Everything is a file** - Including `/proc`, PCI devices, and kernel state
- **Microkernel purity** - Kernel knows nothing; modules provide everything
- **Unix compatibility** - Standard paths (`/dev/hda`, `/proc/self`, `/dev/random`)

## Features

### Current Status

- **ELF module loader** - ET_REL kernel modules loaded by bootloader (ia32)
- **Dynamic VFS** - Full virtual filesystem with mounts, symlinks, and lazy resolution
- **Device support** - `/dev/null`, `/dev/random`, `/dev/tty0`, `/dev/pci`
- **IDE driver** - Works on real hardware**
- **PCI enumerator** - Detects host bridge, ISA, IDE, VGA, Ethernet
- **FAT filesystem** - WIP Basic FAT12/16/32 support (module)
- **Round-robin scheduler** - Processes as VNodes under `/proc`
- **Modular architecture** - Drivers and filesystems as loadable `.ko` modules
- **Real hardware boot** - for ia32, it boots under grub via USB or any grub capable device.

### Works On

- **x86 (ia32)** - QEMU and real hardware **
- **ARMv7** - Port in progress
- **RISC-V 64** - Port in progress

## TODO
### Critical
- [ ] **Paging** - Unimplemented for all architectures
- [ ] **Proper Allocation** - `mmap`, `mfree`, `kmalloc`, `kfree` (currently no-free bump allocator)
- [ ] **UserSpace** - Completely unimplemented
- [ ] **Syscalls** - Linux-compatible syscalls via `$(ARCH)/syscall.c`

### Important
- [ ] **Migrate ia32 → amd64** - For modernity and multicore support
- [ ] **ELF Modules** - Currently IA32-only despite being in `src/kernel/`
- [ ] **Partition support** - `/dev/hda1`, `/dev/hda2`
- [ ] **Better Process creation** - `fork()`, `exec()`

### Nice to Have
- [ ] **Networking stack** - TCP/IP as module
- [ ] **AHCI driver** - Modern SATA support
- [ ] **SMP** - Multicore scheduling

## Building

```bash
make clean all run ARCH=ia32
```