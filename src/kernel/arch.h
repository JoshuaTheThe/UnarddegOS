
#ifndef ARCH_H
#define ARCH_H

#include <_arch.h>

void ArchInitialise(unsigned int a, unsigned int b); // init for that architecture, for instance the IDT/GDT setup for ia32
char *ArchIdentify(void); // return a static string of information
void ArchCli(void); // disable interrupts, or any other thing that may cause CPU state to change other than reset
void ArchSti(void); // enable interrupts, or any other thing that may cause CPU state to change
void Delay(unsigned long ticks);
int ArchGetTemperatureMC(void);
void ArchPause(void);
void LoadModules(unsigned int a, unsigned int b);

#endif
