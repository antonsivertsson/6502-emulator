#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_SIZE 1 << 16     // We have an address bus of 16 bits

extern unsigned char memory[MEMORY_SIZE];

unsigned char getMemoryAtAddress(unsigned short address);
int loadRAM(unsigned char newMemory[MEMORY_SIZE]);
int resetRAM();

#endif // MEMORY_H