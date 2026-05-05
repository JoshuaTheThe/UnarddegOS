
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
                SerialPrint("\n        /\\_/\\\n");
                SerialPrint("       | o o |\n");
                SerialPrint("/-----/-------\\------\\  UnarddegOS\n");
                SerialPrint("|    This Is The,    |\n");
                SerialPrint("| The The The The :3 |\n");
                SerialPrint("|              -josh |\n");
                SerialPrint("\\--------------------/\n\n");
        }
        else if (!strncmp(cmd[0], "cd", 2) && argc == 2)
        {
                chdir(cmd[1]);
        }
        else if (!strncmp(cmd[0], "ls", 2) && argc == 2)
        {
                VNode *Ele = cdir()->FirstChild;
                while (Ele)
                {
                        SerialPrint("%s\n", Ele->Name.Name);
                        Ele = Ele->Next;
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
