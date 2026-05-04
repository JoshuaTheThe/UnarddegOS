
#include <panic.h>
#include <vfs/vfd.h>
#include <string.h>
#include <vmem/alloc.h>
#include <sched/core.h>

static _FileDescriptor *findb(FileDescriptor Index)
{
        _FileDescriptor *Desc = &((Task *)CurrentProc->DriverData)->Files;
        while (Desc->Next)
        {
                if (Desc->Next->FileIndex == Index)
                        return Desc;
                Desc = Desc->Next;
        }
        Panic(PANIC_FD_NOT_FOUND);
}

static _FileDescriptor *find(FileDescriptor Index)
{
        _FileDescriptor *Desc = &((Task *)CurrentProc->DriverData)->Files;
        while (Desc)
        {
                if (Desc->FileIndex == Index)
                        return Desc;
                Desc = Desc->Next;
        }
        Panic(PANIC_FD_NOT_FOUND);
}

static FileDescriptor CreateEntry(VNode *Node)
{
        static FileDescriptor Current = 0;
        Node->Flags |= VFS_OPENED;
        // panics on fail, no null check needed
        _FileDescriptor *FileDesc = kalloc(sizeof(*FileDesc));
        FileDesc->Reference = Node;
        FileDesc->FileIndex = Current;
        FileDesc->Next = NULL;
        if (((Task *)CurrentProc->DriverData)->Files.Next == NULL)
                ((Task *)CurrentProc->DriverData)->Files.Next = FileDesc;
        else
        {
                _FileDescriptor *Desc = &((Task *)CurrentProc->DriverData)->Files;
                while (Desc->Next != NULL)
                {
                        Desc = Desc->Next;
                }
                Desc->Next = FileDesc;
        }

        return Current++;
}

long lseek(FileDescriptor fd, long Offset, int Whence)
{
        _FileDescriptor *File = find(fd);
        VNodeSeek(File->Reference, Offset, Whence);
        return File->Reference->FileOffset;
}

FileDescriptor open(char *const PathFromRoot, VNodeFlags Flags)
{
        (void)Flags;
        PanicIfNull(PathFromRoot);
        VNode *Root = RootVNode();
        VNode *Node = Root->RelativeFind(Root, PathFromRoot, strnlen(PathFromRoot, 256));
        PanicIfNull(Node);
        if (Node->Flags & VFS_OPENED)
                Panic(PANIC_DOUBLE_OPEN);
        return CreateEntry(Node);
}

void close(FileDescriptor fd)
{
        _FileDescriptor *Before = findb(fd);
        _FileDescriptor *Target = Before->Next;
        Before->Next = Target->Next;
        Target->Reference->Flags &= ~VFS_OPENED;
        if (Target->Reference->Flags & VFS_DELETE)
                DeleteVNode(Target->Reference);
        kfree(Target);
}

unsigned long write(FileDescriptor fd,
                    void *const Buf,
                    unsigned long Count)
{
        PanicIfNull(Buf);
        _FileDescriptor *File = find(fd);
        return File->Reference->WriteFunction(Buf, Count, 1, File->Reference);
}

unsigned long read(FileDescriptor fd,
                   void *const Buf,
                   unsigned long Count)
{
        PanicIfNull(Buf);
        _FileDescriptor *File = find(fd);
        return File->Reference->ReadFunction(Buf, Count, 1, File->Reference);
}

void chdir(const char *path)
{
        Task *TaskState = CurrentProc->DriverData;
        TaskState->BaseDir = RootVNode()->RelativeFind(TaskState->BaseDir, path, strnlen(path, 256));
}

VNode *cdir(void)
{
        Task *TaskState = CurrentProc->DriverData;
        return TaskState->BaseDir;
}
