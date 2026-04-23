
#ifndef DEV_NULL_H
#define DEV_NULL_H

#include <vfs/vnode.h>

VNode *CreateNullDevice(const char *const Name, unsigned long NameLength, VNode *Devices);

#endif
