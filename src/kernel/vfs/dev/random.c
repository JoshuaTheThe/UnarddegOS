#include <vfs/dev/random.h>
#include <panic.h>
#include <string.h>

// TODO - use a more random random lol (e.g. +ArchRandomInteger in $ARCH/arch.c)

static uint32_t rng_state = 0xDEADBEEF;

static uint32_t xorshift32(void)
{
        uint32_t x = rng_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        rng_state = x;
        return x;
}

static void mix_entropy(void)
{
        static uint32_t counter = 0;
        extern uint64_t Ticks;
        rng_state ^= counter++ ^ Ticks;
}

static int RandomReadFunction(void *const Buf,
                              const unsigned long Size,
                              const unsigned long Elements,
                              VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);

        unsigned long Bytes = Size * Elements;
        uint8_t *byte_buf = (uint8_t *)Buf;
        mix_entropy();
        for (unsigned long i = 0; i < Bytes; i++)
        {
                if ((i & 3) == 0)
                {
                        uint32_t r = xorshift32();
                        byte_buf[i] = r & 0xFF;
                        byte_buf[i + 1] = (r >> 8) & 0xFF;
                        byte_buf[i + 2] = (r >> 16) & 0xFF;
                        byte_buf[i + 3] = (r >> 24) & 0xFF;
                }
        }

        return Elements;
}

static int RandomWriteFunction(void *const Buf,
                               const unsigned long Size,
                               const unsigned long Elements,
                               VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        uint8_t *byte_buf = (uint8_t *)Buf;
        unsigned long Bytes = Size * Elements;
        for (unsigned long i = 0; i < Bytes; i++)
        {
                rng_state ^= byte_buf[i] << ((i & 31) * 8);
        }

        return Elements;
}

VNode *CreateRandomDevice(const char *const Name,
                          unsigned long NameLength,
                          VNode *Devices)
{
        PanicIfNull(Name);
        PanicIfNull(Devices);
        VNode *NewRandomDevice = NewVNode(VFS_WRITE | VFS_READ);
        NewRandomDevice->Name.Name = Name;
        NewRandomDevice->Name.Length = NameLength;
        NewRandomDevice->ReadFunction = RandomReadFunction;
        NewRandomDevice->WriteFunction = RandomWriteFunction;
        RegisterChildVNode(Devices, NewRandomDevice);
        return NewRandomDevice;
}
