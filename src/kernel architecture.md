
# Syscall Convention

While this project aims to be linux compatible, i will define a custom Cross Architecture calling convention and API numbers.
as this is intended to take "Everything is But a FILE" to the extreme.

## Convention
simply use the first scratch register as the code, and the next 4 for arguments
if the architecture has less than five registers, use the first scratch as a pointer to such a structure
```rust
struct
{
        Code: u...,
        Args: [u<ptr size in bits>; 4];
}
```

## Numbering

| Code (hex) | Name | Kernel Function | Kernel Side Implemented? |
|-|-|-|-|
| `0` | `open` | `open` | Yes. |
| `1` | `close` | `close` | Yes. |
| `2` | `read` | `read` | Yes. |
| `3` | `write` | `write` | Yes. |
| `4` | `vfslink` | `VFSLink` | No. |
| `5` | `vfsunlink` | `VFSUnLink` | No. |
| `6` | `vfsmount` | `VFSMount` | No. |
| `7` | `vfsunmount` | `VFSUnMount` | No. |
| `8` | `seek` | `VFSSeek` | No. |
| `9` | `stat` | `VFSStatus` | No. |
| `a` | `chdir` | `ProcessChangeRelativeDirectory` | No. |
| `b` | `rmdir`,`rmfil` | `VFSRemove` | No. |
| `c` | `chperms` | `VFSChangePermissions` | No. |
| `d` | `fork` | `ProcessFork` | No. |
| `e` | `exec` | `ProcessFromPath` | No. |
| `f` | `pipe` | `OverrideProcessInput` | No. |

### MMap and MUnMap
- these are done using `/sys/mem`, write to free, read to allocate

# Special Files

| Path | Function | Read Only? |
|-|-|-|
| `/sys/mem` | Memory Allocator, write pointer to free, read n bytes to allocate n bytes | F |
| `/sys/time` | Current Time | T |
| `/sys/ticks` | Current Kernel Tick Count | T |
| `/sys/arch` | Architecture Information | T |
| `/sys/mach` | Machine Information | T |
| `/sys/sysinfo` | System Information | T |
| `/dev/stdout` | Standard output for process, proc side link to `/proc/pid/stdio` | F |
| `/dev/stdin` | Standard input for process, proc side link to `/proc/pid/stdio` | F |
| `/dev/stderr` | Standard errror output for process, proc side link to `/proc/pid/stderr` | F |
| `/proc/count` | Count of Processes | T |
| `/proc/self` | proc side link to `/proc/pid` | T |
| `/proc/pid` | proc side containing own pid | T |
| `/proc/ppid` | proc side containing parent's pid | T |
| `/proc/[0-9][0-9]*` | Process | F* |
| `/proc/[0-9][0-9]*/flags` | Process Flags | F* |
| `/proc/[0-9][0-9]*/pid` | Pid | F* |
| `[path]/~next` | Next Sibling | F |
| `[path]/~previous` | Previous Sibling | F |
| `[path]/~first` | First Child | F |
| `[path]/~last` | Last Child | F |
| `[path]/~parent` | Parent File | F |
| `[path]/~self` | echo Self | F |

#### Process Flags

| Bit | Meaning |
| - | - |
| `0` | Paused? |
| `1` | User? (protected) |

# Semantics for translation to disk
- as dirs can hold data, this is required.

if it has children, declare as a directory for that file system, then, create a file named "~self" inside it, which holds it's data.

```c
// iterate a file
int base = open("/path/to/thing", 0);
chdir(base); // unlike UNIX, we use an fd
int fd = open("~first",0); // os doesn't care about flags currently
do
{
        close(fd);
        fd = open("~next", 0);
}
while (fd != -1);
close(base);
```
