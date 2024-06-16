#include "memory.h"

unsigned char memory[MEMORY_SIZE];     // We define our memory as a simple array of 8 bit numbers

unsigned char getMemoryAtAddress(unsigned short address) {
  return memory[address];
}

int loadRAM(unsigned char newMemory[MEMORY_SIZE]) {
  int i;
  for (i = 0; i < MEMORY_SIZE; i++) {
    memory[i] = newMemory[i];
  }
  return 0;
}

int resetRAM() {
  int i;
  for (i = 0; i < MEMORY_SIZE; i++) {
    memory[i] = 0x00;
  }
  return 0;
}