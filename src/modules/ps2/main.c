
#include <ps2/main.h>
#include <drivers/serial.h>
#include <vfs/dev/serial.h>
#include <arch.h>
#include <panic.h>
#include <vmem/alloc.h>

uint8_t packet_data[4];
uint8_t packet_index = 0;
uint8_t packet_size = 3;
uint32_t shifted = 0;
uint32_t ctrl = 0;

/* magic nums oh no */
uint8_t keyboard_map[256] =
    {
        0x00, '\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0x80, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0x00, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

uint8_t keyboard_map_shifted[256] =
    {
        0x00, '\e', '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0x80, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', 0x00, '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, '/', '7',
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

uint8_t ps2_read_status(void)
{
        uint8_t status;
        __asm volatile("inb %1, %0" : "=a"(status) : "Nd"(PS2_STATUS_PORT));
        return status;
}

uint8_t ps2_read_data(void)
{
        while (!(ps2_read_status() & PS2_STATUS_OUTPUT_FULL))
        {
        }

        uint8_t data;
        __asm volatile("inb %1, %0" : "=a"(data) : "Nd"(PS2_DATA_PORT));
        return data;
}

void ps2_write_data(uint8_t data)
{
        while (ps2_read_status() & PS2_STATUS_INPUT_FULL)
        {
        }

        __asm volatile("outb %0, %1" : : "a"(data), "Nd"(PS2_DATA_PORT));
}

void ps2_write_command(uint8_t command)
{
        while (ps2_read_status() & PS2_STATUS_INPUT_FULL)
        {}

        __asm volatile("outb %0, %1" : : "a"(command), "Nd"(PS2_COMMAND_PORT));
}

bool ps2_wait_output(void)
{
        for (int i = 0; i < 10000; i++)
        {
                if (ps2_read_status() & PS2_STATUS_OUTPUT_FULL)
                {
                        return true;
                }
        }
        return false;
}

bool ps2_wait_input(void)
{
        while (ps2_read_status() & PS2_STATUS_INPUT_FULL)
        {
                __asm volatile("nop");
        }
        return true;
}

bool ps2_send_device_command(uint8_t port, uint8_t command)
{
        if (port == 2)
        {
                ps2_write_command(PS2_CMD_WRITE_PORT2);
        }

        ps2_write_data(command);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t response = ps2_read_data();
        return (response == PS2_RESP_ACK);
}

void ps2_mouse_fetch(int *mx, int *my, uint8_t *buttons)
{
        ps2_mouse_packet_t packet;

        while (ps2_read_mouse_packet(&packet))
        {
                int delta_x = (int8_t)packet.x_movement;
                int delta_y = (int8_t)packet.y_movement;
                *mx += delta_x;
                *my -= delta_y;
                *buttons = packet.buttons;
                break;
        }
}

bool ps2_mouse_present(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT2);
        for (volatile size_t i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_command(PS2_CMD_READ_CONFIG);
        if (!ps2_wait_output())
        {
                return false;
        }

        ps2_write_command(PS2_CMD_READ_CONFIG);
        uint8_t config = ps2_read_data();
        config |= (1 << 5);
        ps2_write_command(PS2_CMD_WRITE_CONFIG);
        ps2_write_data(config);

        if (!(config & (1 << 5)))
        {
                return false;
        }

        ps2_write_command(PS2_CMD_TEST_PORT2);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t test_result = ps2_read_data();
        if (test_result != 0x00)
        {
                return false;
        }

        if (!ps2_send_device_command(2, PS2_DEV_IDENTIFY))
        {
                return false;
        }

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t id1 = ps2_read_data();
        uint8_t id2 __attribute__((unused)) = 0x00;
        if (ps2_wait_output())
        {
                id2 = ps2_read_data();
        }

        if (id1 == MOUSE_STANDARD || id1 == MOUSE_HAS_SCROLL || id1 == MOUSE_HAS_5_BUTTONS)
        {
                return true;
        }

        return false;
}

void ps2_initialize_mouse(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT2);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        // Reset mouse
        ps2_send_device_command(2, 0xFF);
        for (volatile int i = 0; i < 100000; i++)
                __asm volatile("nop");

        // Enable data reporting
        ps2_send_device_command(2, 0xF4);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        // Set sample rate to something reasonable
        ps2_send_device_command(2, 0xF3);
        ps2_send_device_command(2, 100);
}

bool ps2_read_mouse_packet(ps2_mouse_packet_t *packet)
{
        ArchCli();

        if (!(ps2_read_status() & PS2_STATUS_OUTPUT_FULL))
                return false;

        uint8_t status = ps2_read_status();
        if (!(status & (1 << 5)))
        {
                return false;
        }

        uint8_t data = ps2_read_data();

        if (packet_index == 0)
        {
                if (!(data & (1 << 3)))
                {
                        packet_index = 0;
                        return false;
                }
        }

        packet_data[packet_index++] = data;

        if (packet_index >= packet_size)
        {
                packet_index = 0;

                packet->flags = packet_data[0];
                packet->x_movement = packet_data[1];
                packet->y_movement = packet_data[2];

                if (packet_data[0] & (1 << 6))
                        packet->x_movement = 0;
                if (packet_data[0] & (1 << 7))
                        packet->y_movement = 0;

                if (packet_data[0] & (1 << 4))
                        packet->x_movement |= 0xFFFFFF00;
                if (packet_data[0] & (1 << 5))
                        packet->y_movement |= 0xFFFFFF00;

                packet->buttons = 0;
                if (packet_data[0] & (1 << 0))
                        packet->buttons |= MOUSE_LEFT_BUTTON;
                if (packet_data[0] & (1 << 1))
                        packet->buttons |= MOUSE_RIGHT_BUTTON;
                if (packet_data[0] & (1 << 2))
                        packet->buttons |= MOUSE_MIDDLE_BUTTON;

                packet->z_movement = 0;

                return true;
        }

        ArchSti();
        return false;
}

uint8_t ps2_getchar(void)
{
        bool hit = false;
        char character = 0;
        while (!hit)
        {
                character = ps2_keyboard_fetch(&hit);
        }
        return character;
}

uint8_t ps2_keyboard_fetch(volatile bool *hit)
{
        ArchCli();
        if (hit)
                *hit = false;
        if (!(inb(0x64) & 0x01))
        {
                ArchSti();
                return 0;
        }

        uint8_t status = inb(0x64);
        if (status & (1 << 5))
        {
                (void)inb(0x60);
                ArchSti();
                return 0;
        }

        uint16_t scancode = inb(0x60);
        ArchSti();
        if (scancode == 0x2a || scancode == 0x36)
        {
                shifted = 1;
                return 0;
        }
        else if (scancode == 0xaa || scancode == 0xb6)
        {
                shifted = 0;
                return 0;
        }
        if (scancode == 0x1d)
        {
                ctrl = 1;
                return 0;
        }
        else if (scancode == 0x9d)
        {
                ctrl = 0;
                return 0;
        }
        if (scancode & 0x80)
        {
                return 0;
        }
        if (scancode >= 128)
        {
                return 0;
        }

        uint8_t key = shifted ? keyboard_map_shifted[scancode] : keyboard_map[scancode];
        if (hit)
                *hit = true;
        if (ctrl)
        {
                if (key >= 'a' && key <= 'z')
                {
                        return key - 'a' + 1;
                }
                else if (key >= 'A' && key <= 'Z')
                {
                        return key - 'A' + 1;
                }
        }
        return key;
}

void ps2_initialize_keyboard(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT1);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_data(0xFF);
        for (volatile int i = 0; i < 100000; i++)
                __asm volatile("nop");
        if (!ps2_wait_output())
                return;
        uint8_t reset_response = ps2_read_data();
        if (reset_response != 0xAA && reset_response != 0xFA)
        {
                SerialPrint(" [Error] Keyboard reset failed: 0x%x\n", reset_response);
                return;
        }
        while (ps2_wait_output())
                ps2_read_data();
        ps2_write_data(0xF0);
        ps2_write_data(0x02);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_data(0xF4);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_data(0xF3);
        ps2_write_data(0x20);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
}

bool ps2_keyboard_present(void)
{
        while (ps2_read_status() & PS2_STATUS_OUTPUT_FULL)
        {
                ps2_read_data();
        }

        ps2_write_data(0xEE);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t response = ps2_read_data();
        if (response != 0xEE)
        {
                return false;
        }

        ps2_write_data(0xFF);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t ack = ps2_read_data();
        if (ack != 0xFA)
        {
                return false;
        }

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t reset_code = ps2_read_data();
        if (reset_code != 0xAA)
        {
                return false;
        }

        return true;
}

int ps2_keyboard_read(void *const Buf, const unsigned long Size, const unsigned long Elements, VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        char *ByteBuf = (char *)Buf;
        unsigned long i;
        const unsigned long Bytes = Size * Elements;
        for (i = 0; i < Bytes; ++i)
        {
                char chr = ps2_getchar();
                if (chr == '\r' && (*(TTYFlags *)Node->DriverData) & TTY_CRNL)
                        chr = '\n';
                if ((*(TTYFlags *)Node->DriverData) & TTY_ECHO)
                {
                        Node->WriteFunction(&chr, 1, 1, Node);
                }
                if (!((*(TTYFlags *)Node->DriverData) & TTY_RAW))
                {
                        if (chr == '\b' && i > 0)
                        {
                                i -= 2;
                                ByteBuf[i] = 0;
                                continue;
                        }
                }
                ByteBuf[i] = chr;
                if (chr == '\n' && (*(TTYFlags *)Node->DriverData) & TTY_COOKED)
                        break;
        }

        return i / Size;
}

int ps2_mouse_read(void *const Buf, const unsigned long Size, const unsigned long Elements, VNode *const Node)
{
        PanicIfNull(Buf);
        PanicIfNull(Node);
        char *ByteBuf = (char *)Buf;
        unsigned long i;
        const unsigned long Bytes = Size * Elements;
        ps2_mouse_packet_t packet = {0};
        while (!ps2_read_mouse_packet(&packet))
        {
                return 0;
        }
        
        for (i = 0; i < Bytes && i < sizeof(ps2_mouse_packet_t); ++i)
        {
                ByteBuf[i] = ((char *)&packet)[i];
        }
                
        return i / Size;
}

int Init(void)
{
        ArchCli();
        VNode *Dev = RootVNode()->RelativeFind(RootVNode(), "/dev", 4);
        if (ps2_keyboard_present())
        {
                ps2_initialize_keyboard();
                VNode *Keyboard       = NewVNode(VFS_READ);
                Keyboard->Name.Name   = "ps2_kbd";
                Keyboard->Name.Length = 7;
                Keyboard->ReadFunction= &ps2_keyboard_read;
                Keyboard->DriverData = kalloc(sizeof(TTYFlags));
                *(TTYFlags *)Keyboard->DriverData = TTY_COOKED | TTY_ECHO | TTY_NLCR | TTY_CRNL;
                RegisterChildVNode(Dev, Keyboard);
        }

        if (ps2_mouse_present())
        {
                ps2_initialize_mouse();
                VNode *Mouse       = NewVNode(VFS_READ);
                Mouse->Name.Name   = "ps2_mouse";
                Mouse->Name.Length = 9;
                Mouse->ReadFunction= &ps2_mouse_read;
                RegisterChildVNode(Dev, Mouse);
        }
        ArchSti();
        return 0;
}
