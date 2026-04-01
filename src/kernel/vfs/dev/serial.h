
#ifndef DEV_SERIAL_H
#define DEV_SERIAL_H

#include <vfs/vnode.h>

VNode *CreateSerialDevice(const char *const Name, unsigned long NameLength, VNode *Devices);

#endif

