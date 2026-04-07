#ifndef COMMON_SERIAL_H
#define COMMON_SERIAL_H

#include<stdint.h>
#include<stdbool.h>
#include<io.h>

void SerialInit(void);
void SerialPut(char c);
void SerialPrint(const char *str,...);
bool SerialCanRead(void);
char SerialRead(void);
void SerialWaitForTransmit(void);
void SerialWaitForInput(void);

#endif
