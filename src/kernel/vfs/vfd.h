
// simply converts the tree to a flat linked list when needed
// so fd lookup is O(opened_files) not like O(n!!!!!)
// files are appended to the end when opened

#ifndef VFD_H
#define VFD_H

#include <stdint.h>
#include <stddef.h>
#include <vfs/vnode.h>

typedef unsigned long FileDescriptor;

typedef struct _FileDescriptor
{
        VNode                  *Reference;
        struct _FileDescriptor *Next;
        FileDescriptor          FileIndex;
} _FileDescriptor;

FileDescriptor open(char *const PathFromRoot, VNodeFlags Flags);
void close(FileDescriptor fd);
unsigned long write(FileDescriptor fd,
                    void *const Buf,
                    unsigned long Count);
unsigned long read(FileDescriptor fd,
                   void *const Buf,
                   unsigned long Count);
long lseek(FileDescriptor fd,
           long Offset, int Whence);

void chdir(const char *path);
VNode *cdir(void);

#endif

