
#ifndef DEV_SERIAL_H
#define DEV_SERIAL_H

#include <vfs/vnode.h>

#define TTY_RAW              (1 << 0)  /* pass bytes through unmodified */
#define TTY_COOKED           (1 << 1)  /* line buffered, process special chars */
#define TTY_ECHO             (1 << 2)  /* echo input back to output */
#define TTY_CRNL             (1 << 3)  /* translate \r -> \n on input */
#define TTY_NLCR             (1 << 4)  /* translate \n -> \r\n on output */

typedef unsigned long TTYFlags;

VNode *CreateSerialDevice(const char *const Name, unsigned long NameLength, VNode *Devices);

#endif

