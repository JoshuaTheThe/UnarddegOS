
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <vmem/alloc.h>
#include <vfs/vfd.h>
#include <string.h>
#include <drivers/serial.h>

#define CMDM_LEN 64
#define MARGS 32

VNode *CurrentNode;

int Parse(char *b, char cmd[MARGS][CMDM_LEN])
{
        int N = 0, i = 0;

        while (*b && N < MARGS)
        {
                while (*b == ' ')
                        b++;
                if (!*b)
                        break;

                i = 0;

                if (*b == '"')
                {
                        b++;
                        while (*b && *b != '"' && i < CMDM_LEN - 1)
                        {
                                if (*b == '\\' && *(b + 1))
                                {
                                        b++;
                                        if (*b == 'n')
                                                cmd[N][i++] = '\n';
                                        else if (*b == 'r')
                                                cmd[N][i++] = '\r';
                                        else if (*b == 't')
                                                cmd[N][i++] = '\t';
                                        else if (*b == 'a')
                                                cmd[N][i++] = '\a';
                                        else if (*b == 'v')
                                                cmd[N][i++] = '\v';
                                        else
                                                cmd[N][i++] = *b;
                                }
                                else
                                {
                                        cmd[N][i++] = *b;
                                }
                                b++;
                        }
                        if (*b == '"')
                                b++;
                }
                else
                {
                        while (*b && *b != ' ' && i < CMDM_LEN - 1)
                                cmd[N][i++] = *b++;
                }

                cmd[N][i] = '\0';
                N++;
        }

        return N;
}

bool SystemCommand(char *sys)
{
        char cmd[MARGS][CMDM_LEN];
        memset(cmd, 0, sizeof(cmd));
        int argc = Parse(sys, (char(*)[CMDM_LEN]) & cmd);
        if (!strncmp(cmd[0], "goober", 6))
        {
                SerialPrint("\r\n        /\\_/\\\r\n");
                SerialPrint("       | o o |\r\n");
                SerialPrint("/-----/-------\\------\\  UnarddegOS\r\n");
                SerialPrint("|    This Is The,    |\r\n");
                SerialPrint("| The The The The :3 |\r\n");
                SerialPrint("|              -josh |\r\n");
                SerialPrint("\\--------------------/\r\n\r\n");
        }
        else if (!strncmp(cmd[0], "cd", 3) && argc == 2)
        {
                chdir(cmd[1]);
        }
        else if (!strncmp(cmd[0], "ls", 3) && argc == 2)
        {
                VNode *Ele = cdir()->RelativeFind(cdir(), cmd[1], strnlen(cmd[1], 1024))->FirstChild;
                while (Ele)
                {
                        SerialPrint("%s\r\n", Ele->Name.Name);
                        Ele = Ele->Next;
                }
        }
        else if (!strncmp(cmd[0], "type", 5) && argc == 2)
        {
                VNode *Fil = cdir()->RelativeFind(cdir(), cmd[1], strnlen(cmd[1], 1024));
                char chr = 0;
                while (Fil->ReadFunction(&chr, 1, 1, Fil) == 1 && chr)
                {
                        SerialPrint("%c", chr);
                }
        }
        else if (!strncmp(cmd[0], "exit", 4))
        {
                return false;
        }

        return true;
}

int Init(void)
{
        char Buffer[256];
        CurrentNode = RootVNode();
        FileDescriptor fd = open("/dev/tty0", 0);
        SystemCommand("goober");
        bool running = true;
        while (running)
        {
                memset(Buffer, 0, 256);
                SerialPrint("\\`(owo`)o -> ");
                read(fd, &Buffer, sizeof(Buffer));
                running = SystemCommand(Buffer);
        }

        close(fd);
        return 0;
}
