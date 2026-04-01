
#include <panic.h>
#include <vmem/bumpalloc.h> // replace with kernel allocator in future
#include <vfs/vnode.h>
#include <string.h>

VNode Root = {0};

// to prevent edge cases
// for directories // drives, this would read from the RAW drive or error on dir
static void DefaultReadFunction(void *const Buf,
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
}

// for directories // drives, this would write to the RAW drive or error on dir
static void DefaultWriteFunction(void *const Buf,
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
        for (unsigned long i = 0; i < RelativePathLength; ++i)
        {
                PanicIfNull(CurrentNode);
                unsigned long TempLength = 0;
                // Copy until a seperator charater
                for (unsigned long j = 0; j < sizeof(TempBuffer) - 1 && i + j < RelativePathLength; ++j)
                {
                        TempLength += 1;
                        TempBuffer[j] = TempBuffer[j + 1] = 0;
                        if (RelativePath[i+j] == '/')
                        {
                                if (TempLength == 1)
                                        TempBuffer[0] = '/';
                                TempLength -= 1;
                                break;
                        }
                        TempBuffer[j] = RelativePath[i+j];
                }

                // Update by N bytes
                if (TempLength != 0)
                {
                        i += TempLength;
                }

                // Traverse to Parent
                if (TempLength == 2 && TempBuffer[1] == '.' && TempBuffer[0] == '.')
                {
                        if (CurrentNode != RootVNode())
                                CurrentNode = CurrentNode->Parent;
                }
                // Traverse to Self
                else if (TempLength == 1 && TempBuffer[0] == '.')
                {;}
                // Traverse to Root
                else if (TempLength == 0 && TempBuffer[0] == '/')
                {
                        CurrentNode = RootVNode();
                }
                else
                {
                        VNode *Child = CurrentNode->FirstChild;
                        while (Child != NULL)
                        {
                                if (TempLength == Child->Name.Length &&
                                    !unsafe_strncmp(Child->Name.Name, TempBuffer, Child->Name.Length))
                                {
                                        CurrentNode = Child;
                                        break;
                                }
                                Child = Child->Next;
                        }
                        if (Child == NULL)
                        {
                                Panic(PANIC_NOT_FOUND);
                        }
                }
                TempLength = 0;
        }
        return CurrentNode;
}

void VNodeDefault(VNode *Node)
{
        PanicIfNull(Node);
        static char *const DefaultName = "~UnknownVNode";
        Node->ReadFunction             = DefaultReadFunction;
        Node->WriteFunction            = DefaultWriteFunction;
        Node->DestroyChildren          = DefaultDestroyChildrenFunction;
        Node->ConstructChildren        = DefaultConstructChildrenFunction;
        Node->RelativeFind             = DefaultRelativeFindFunction;
        Node->Name.Name                = DefaultName;
        Node->Name.Length              = sizeof(DefaultName);
        Node->Link                     = NULL;
        Node->Parent                   = NULL;
        Node->Next                     = NULL;
        Node->Previous                 = NULL;
        Node->FirstChild               = NULL;
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
