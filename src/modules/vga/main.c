#include <vfs/vnode.h>
#include <vfs/vfd.h>
#include <arch.h>
#include <panic.h>
#include <vfs/dev/serial.h>
#include <string.h>

struct multiboot_tag_framebuffer
{
        uint16_t type;
        uint16_t flags;
        uint32_t size;
        uint64_t framebuffer_addr;
        uint32_t framebuffer_pitch;
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        uint8_t framebuffer_bpp;
        uint8_t framebuffer_type;
        uint16_t reserved;
} __attribute__((packed));

uintptr_t FrameBufferAddress;
uint32_t FrameBufferWidth;
uint32_t FrameBufferHeight;
uint32_t FrameBufferPitch;
uint8_t FrameBufferBPP;

void ParseMBI(void *mbi_addr)
{
        unsigned int offset = 8;
        struct multiboot_tag *tag;
        while (1)
        {
                tag = (struct multiboot_tag *)(mbi_addr + offset);
                if (tag->type == 0)
                        break;
                if (tag->type == 8)
                {
                        struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer *)tag;
                        FrameBufferAddress = (uintptr_t)fb->framebuffer_addr;
                        FrameBufferWidth = fb->framebuffer_width;
                        FrameBufferHeight = fb->framebuffer_height;
                        FrameBufferPitch = fb->framebuffer_pitch;
                        FrameBufferBPP = fb->framebuffer_bpp;
                        SerialPrint(" [Info] Framebuffer: addr=0x%x, %dx%d, pitch=%d, bpp=%d\r\n",
                                    FrameBufferAddress, FrameBufferWidth, FrameBufferHeight,
                                    FrameBufferPitch, FrameBufferBPP);
                }

                offset += (tag->size + 7) & ~7;
        }
}

int Write(void *const Buf, const unsigned long Size, const unsigned long Elements, VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        const unsigned long Bytes = Size * Elements;
        memcpy((void *)(FrameBufferAddress + Node->FileOffset), Buf, Bytes);
        Node->FileOffset += Bytes;
        return Elements;
}

int Read(void *const Buf, const unsigned long Size, const unsigned long Elements, VNode *const Node)
{
        (void)Buf;
        (void)Size;
        (void)Elements;
        (void)Node;
        Panic(PANIC_TODO);
}

int Init(void)
{
        VNode *Dev = RootVNode()->RelativeFind(RootVNode(), "/dev", 4);
        VNode *VGA = NewVNode(VFS_READ | VFS_WRITE);
        VGA->Name.Name = "framebuffer";
        VGA->Name.Length = 11;
        VGA->WriteFunction = Write;
        VGA->ReadFunction = Read;
        ParseMBI(RootVNode()->RelativeFind(RootVNode(), "/sys/mbi", 8)->DriverData);
        RegisterChildVNode(Dev, VGA);
        return 0;
}
