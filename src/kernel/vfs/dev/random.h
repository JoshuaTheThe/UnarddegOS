#ifndef RANDOM_H
#define RANDOM_H

#include <vfs/vnode.h>

VNode *CreateRandomDevice(const char *const Name, 
                          unsigned long NameLength, 
                          VNode *Devices);

#endif
