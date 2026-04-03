
#include <panic.h>
#include <vfs/vfd.h>
#include <string.h>
#include <vmem/bumpalloc.h>

_FileDescriptor *Files = NULL;

static _FileDescriptor *find(FileDescriptor Index)
{
	_FileDescriptor *Desc = Files;
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
	_FileDescriptor *FileDesc = BumpAllocate(sizeof(*FileDesc));
	FileDesc->Reference = Node;
	FileDesc->FileIndex = Current;
	FileDesc->Next = NULL;
	if (Files == NULL)
		Files = FileDesc;
	else
	{
		_FileDescriptor *Desc = Files;
		while (Desc->Next != NULL)
		{
			Desc = Desc->Next;
		}
		Desc->Next = FileDesc;
	}

	return Current++;
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
	CreateEntry(Node);
	return (FileDescriptor)0;
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

