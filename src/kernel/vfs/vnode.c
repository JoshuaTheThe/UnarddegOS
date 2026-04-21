
#include <panic.h>
#include <vmem/bumpalloc.h> // replace with kernel allocator in future
#include <vfs/vnode.h>
#include <string.h>

VNode Root = {0};

void VNodeSeek(VNode *Node,
               long Offset,
               int  Type)
{
        switch (Type)
        {
                case SEEK_SET:
                        Node->FileOffset = Offset;
                        break;
                case SEEK_CUR:
                        Node->FileOffset += Offset;
                        break;
                case SEEK_END:
                        Panic(PANIC_TODO);
                        break;
                default:
                        break;
        }
}

// to prevent edge cases
// for directories // drives, this would read from the RAW drive or error on dir
static int DefaultReadFunction(void *const Buf,
                               const unsigned long Size,
                               const unsigned long Elements,
                               VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        (void)Buf;
        (void)Size;
        (void)Elements;
        (void)Node;
        Panic(PANIC_UNINTENDED_CALL);
        return 0;
}

// for directories // drives, this would write to the RAW drive or error on dir
static int DefaultWriteFunction(void *const Buf,
                                const unsigned long Size,
                                const unsigned long Elements,
                                VNode *Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        (void)Buf;
        (void)Size;
        (void)Elements;
        (void)Node;
        Panic(PANIC_UNINTENDED_CALL);
        return 0;
}

// create a node tree of children from disk
static void DefaultConstructChildrenFunction(VNode *Node, unsigned long MaxDepth)
{
        PanicIfNull(Node);
        (void)Node;
        (void)MaxDepth;
        Panic(PANIC_UNINTENDED_CALL);
}

// free said node tree, called when no longer needed
static void DefaultDestroyChildrenFunction(VNode *Node)
{
        PanicIfNull(Node);
        (void)Node;
        Panic(PANIC_UNINTENDED_CALL);
}

// return a VNode relative to the Base, for instance "." is self, ".." would be parent, ...
static VNode *DefaultRelativeFindFunction(VNode *Base, const char *const RelativePath, unsigned long RelativePathLength)
{
        PanicIfNull(Base);
        PanicIfNull(RelativePath);
        static char TempBuffer[128];
        VNode *CurrentNode = Base;

        unsigned long i = 0;
        if (RelativePathLength > 0 && RelativePath[0] == '/')
        {
                CurrentNode = RootVNode();
                i = 1;
        }

        while (i < RelativePathLength)
        {
                PanicIfNull(CurrentNode);
                if (RelativePath[i] == '/')
                {
                        ++i;
                        continue;
                }

                unsigned long TempLength = 0;
                while (i + TempLength < RelativePathLength &&
                       RelativePath[i + TempLength] != '/' &&
                       TempLength < sizeof(TempBuffer) - 1)
                {
                        TempBuffer[TempLength] = RelativePath[i + TempLength];
                        ++TempLength;
                }
                TempBuffer[TempLength] = 0;
                i += TempLength;

                if (TempLength == 0)
                        continue;

                if ((TempLength == 2 && TempBuffer[0] == '.' && TempBuffer[1] == '.') ||
                    (TempLength == 7 && !strncmp(TempBuffer, "~parent", 7)))
                {
                        if (CurrentNode != RootVNode())
                                CurrentNode = CurrentNode->Parent;
                }
                else if ((TempLength == 1 && TempBuffer[0] == '.') ||
                         (TempLength == 5 && !strncmp(TempBuffer, "~self", 5)))
                {
                        /* stay on current node */
                }
                else if (TempLength == 5 && !strncmp(TempBuffer, "~next", 5))
                {
                        if (CurrentNode->Next == NULL)
                                Panic(PANIC_NOT_FOUND);
                        CurrentNode = CurrentNode->Next;
                }
                else if (TempLength == 9 && !strncmp(TempBuffer, "~previous", 9))
                {
                        if (CurrentNode->Previous == NULL)
                                Panic(PANIC_NOT_FOUND);
                        CurrentNode = CurrentNode->Previous;
                }
                else if (TempLength == 6 && !strncmp(TempBuffer, "~first", 6))
                {
                        if (CurrentNode->FirstChild == NULL)
                                Panic(PANIC_NOT_FOUND);
                        CurrentNode = CurrentNode->FirstChild;
                }
                else if (TempLength == 5 && !strncmp(TempBuffer, "~last", 5))
                {
                        if (CurrentNode->FirstChild == NULL)
                                Panic(PANIC_NOT_FOUND);
                        VNode *Last = CurrentNode->FirstChild;
                        while (Last->Next)
                                Last = Last->Next;
                        CurrentNode = Last;
                }
                else
                {
                        VNode *Child = CurrentNode->FirstChild;
                        while (Child != NULL)
                        {
                                if (TempLength == Child->Name.Length &&
                                    !strncmp(Child->Name.Name, TempBuffer, TempLength))
                                {
                                        CurrentNode = Child;
                                        break;
                                }
                                Child = Child->Next;
                        }
                        if (Child == NULL)
                                Panic(PANIC_NOT_FOUND);
                }
        }

        return CurrentNode;
}

void VNodeDefault(VNode *Node)
{
        PanicIfNull(Node);
        static char *const DefaultName = "~UnknownVNode";
        Node->ReadFunction = DefaultReadFunction;
        Node->WriteFunction = DefaultWriteFunction;
        Node->DestroyChildren = DefaultDestroyChildrenFunction;
        Node->ConstructChildren = DefaultConstructChildrenFunction;
        Node->RelativeFind = DefaultRelativeFindFunction;
        Node->Name.Name = DefaultName;
        Node->Name.Length = sizeof(DefaultName);
        Node->Link = NULL;
        Node->Parent = NULL;
        Node->Next = NULL;
        Node->Previous = NULL;
        Node->FirstChild = NULL;
        Node->DriverData = NULL;
}

VNode *NewVNode(VNodeFlags Flags)
{
        // panics on fail, memsets automatically
        VNode *NewNode = BumpAllocate(sizeof(*NewNode));
        NewNode->Flags = Flags;
        VNodeDefault(NewNode);
        return NewNode;
}

void DeleteNode(VNode *Node)
{
        PanicIfNull(Node);
        (void)Node; // bump allocator does not allow resource freeing
        return;
}

void RegisterChildVNode(VNode *const Parent, VNode *const Child)
{
        PanicIfNull(Parent);
        PanicIfNull(Child);
        if (Parent->FirstChild != NULL)
                Parent->FirstChild->Previous = Child;
        Child->Next = Parent->FirstChild;
        Parent->FirstChild = Child;
}

void RegisterSiblingVNode(VNode *const Base, VNode *const Child)
{
        PanicIfNull(Base);
        PanicIfNull(Child);
        if (Base->Next != NULL)
                Base->Next->Previous = Child;
        Child->Next = Base->Next;
        Base->Next = Child;
}

VNode *RootVNode(void)
{
        return &Root;
}

void VNListTree(VNode *Base, int Depth)
{
        while (Base != NULL)
        {
                for (int i = 0; i < Depth - 1; ++i)
                        SerialPut(' ');
                for (unsigned long i = 0; i < Base->Name.Length; ++i)
                        SerialPut(Base->Name.Name[i]);
                SerialPut('\r');
                SerialPut('\n');
                if (Base->FirstChild != NULL)
                        VNListTree(Base->FirstChild, Depth + 1);
                Base = Base->Next;
        }
}
