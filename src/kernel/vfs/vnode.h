
#ifndef VNODE_H
#define VNODE_H

#define VFS_WRITE   (0x01)
#define VFS_READ    (0x02)
#define VFS_HIDDEN  (0x04)
#define VFS_SYSTEM  (0x08)
#define VFS_LINK    (0x10)
#define VFS_MOUNTED (0x20) // treat the linked file as a device // directory

struct VNode;

typedef void (*VNodeRWFunction)(
        void *const Buf,
        const unsigned long Size,
        const unsigned long Elements,
        struct VNode *const Node);

typedef void (*VNodeFunction)(struct VNode *const Node);
typedef void (*VNodeConstructFunction)(struct VNode *const Node, unsigned long MaxDepth);
typedef struct VNode *(*VNodeFindFunction)(struct VNode *Base, const char *const RelativePath);

typedef unsigned char VNodeFlags;

typedef struct
{
        unsigned char Year : 7;
        unsigned char Day : 5;
        unsigned char Month : 4;
} VNodeTime;

typedef struct
{
        const char   *Name;
        unsigned long Length;
} VNodeString;

typedef struct VNode
{
        VNodeString            Name;
        struct VNode          *Parent,
                              *FirstChild,
                              *Next,
                              *Previous,
                              *Link; // Link for say, Mount or SymLink
        VNodeRWFunction        ReadFunction;
        VNodeRWFunction        WriteFunction;
        VNodeConstructFunction ConstructChildren; // for Drives // Directories
        VNodeFunction          DestroyChildren;   // for Drives // Directories
        VNodeFindFunction      RelativeFind;
        VNodeTime              Created;
        VNodeTime              LastModified;
        VNodeTime              LastAccessed;
        VNodeFlags             Flags;
} VNode;

// root will be a node,
// where Root->ConstructChildren
// finds all drives and devices, which may find all files upon asking

VNode *NewVNode(VNodeFlags Flags);
void   DeleteVNode(void);
void   RegisterChildVNode(VNode *const Parent, VNode *const Child);
void   RegisterSiblingVNode(VNode *const Base, VNode *const Child);
VNode *RootVNode(void);
void   VNodeDefault(VNode *Node);
void   VNListTree(VNode *Base, int Depth);

#endif
