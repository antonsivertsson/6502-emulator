#include <stdio.h>
#include "memory.h"
#include "6502.h"

#define FLAG_C 1 << 7       // 0b10000000
#define FLAG_Z 1 << 6       // 0b01000000
#define FLAG_I 1 << 5       // 0b00100000
#define FLAG_D 1 << 4       // 0b00010000
#define FLAG_B 1 << 3       // 0b00001000
#define FLAG_UNUSED 1 << 2  // 0b00000100 - Unused flag, always set, no need for bit field
#define FLAG_V 1 << 1       // 0b00000010
#define FLAG_N 1 << 0       // 0b00000001

// Define CPU
CPU cpu;

// INIT functions
int initFlags() {
  cpu.REG_PS = cpu.REG_PS & 0x00; // Clear the status registry using bitwise AND
  cpu.REG_PS |= FLAG_UNUSED; // Set the unused flag (always set) using bitwise OR
  cpu.REG_PS |= FLAG_D; // Set the decimal flag (set at start) using bitwise OR
  return 0;
}

void setFlag(unsigned char flag_bit_mask) {
  cpu.REG_PS |= flag_bit_mask;
  return;
}

void clearFlag(unsigned char flag_bit_mask) {
  cpu.REG_PS &= ~flag_bit_mask;
  return;
}

// Operations for flags
int modifyNegativeFlag(unsigned char input) {
  // Set if result of previous operation has bit 7 set to 1
  if (input >> 7) { setFlag(FLAG_N); } else { clearFlag(FLAG_N); }
  return input >> 7;
}

int modifyZeroFlag(unsigned char input) {
  if (input == 0x00) { setFlag(FLAG_Z); } else { clearFlag(FLAG_Z); }
  return input == 0x00;
}

int modifyOverflowFlag(unsigned char input1, unsigned char input2, unsigned char result) {
  // See: https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
  int overflow = ((input1 ^ result) & 0x80) & ((input2 ^ result) & 0x80);
  if (overflow) {
    setFlag(FLAG_V);
  } else {
    clearFlag(FLAG_V);
  }
  return overflow;
}

int initProgramCounter() {
  // Set PC to value at reset vector (memory address FFFC-FFFD), in little endian format, so the byte stored at FFFD first
  cpu.REG_PC = memory[0xFFFD] << 8 | memory[0xFFFC];
  return 0;
}

int resetCPU() {
  // Reset all registers
  cpu.REG_PC &= 0x0000;
  cpu.REG_PS &= 0x00;
  cpu.REG_SP &= 0x00;
  cpu.REG_A &= 0x00;
  cpu.REG_X &= 0x00;
  cpu.REG_Y &= 0x00;

  // Set required vals
  initFlags();
  initProgramCounter();

  cpu.isRunning = 1;
  return 0;
}

// utils
unsigned char getMemAddress(Addr_mode addressing_mode, unsigned short operand) {
  // Operand may be 16 bit if e.g. addressing_mode = absolute
  unsigned char mem_address = operand;
  switch (addressing_mode) {
    case absolute:
      mem_address = operand;
      break;
    case absolute_indirect:
      mem_address = memory[operand + 1] << 8 | memory[operand];
      break;
    case absolute_idx_X:
      mem_address = operand + cpu.REG_X;
      break;
    case absolute_idx_Y:
      mem_address = operand + cpu.REG_Y;
      break;
    case relative:
      mem_address = cpu.REG_PC + (unsigned char)operand;
      break;
    case zero_page:
      mem_address = 0x00 << 8 | (unsigned char)operand;
      break;
    case zero_page_idx_indirect_X:
      mem_address = memory[(unsigned char)operand + cpu.REG_X + 1] << 8 | memory[(unsigned char)operand + cpu.REG_X];
      break;
    case zero_page_idx_X:
      mem_address = (unsigned char)operand + cpu.REG_X;
      break;
    case zero_page_idx_Y:
      mem_address = (unsigned char)operand + cpu.REG_Y;
      break;
    case zero_page_indirect_idx_Y:
      mem_address = (memory[(unsigned char)operand + 1] << 8 | memory[(unsigned char)operand]) + cpu.REG_Y;
      break;
    default:
      return -1;
  }
  return mem_address;
}

int load(unsigned char *reg, Addr_mode addressing_mode, unsigned short operand) {
  // Loads the value at address in accordance with the addressing mode
  if (addressing_mode == immediate) {
    *reg = operand;
  }
  else
  {
    unsigned char mem_address = getMemAddress(addressing_mode, operand);
    *reg = memory[mem_address];
  }
  return 0;
}

// Store value at memory address
int store(unsigned char value, Addr_mode addressing_mode, unsigned short operand) {
  unsigned char mem_address = getMemAddress(addressing_mode, operand);
  memory[mem_address] = value;
  return 0;
}

// Goes to a place in memory and increments the value
int incrementInMemory(Addr_mode address_mode, unsigned char operand) {
  unsigned short mem_address = getMemAddress(address_mode, operand);
  memory[mem_address]++;
  unsigned char result = memory[mem_address];
  modifyNegativeFlag(result);
  modifyZeroFlag(result);
  return 0;
}

// Goes to a place in memory and increments the value
int decrementInMemory(Addr_mode address_mode, unsigned char operand) {
  unsigned short mem_address = getMemAddress(address_mode, operand);
  memory[mem_address]--;
  unsigned char result = memory[mem_address];
  modifyNegativeFlag(result);
  modifyZeroFlag(result);
  return 0;
}

// Reads next byte at program counter and increments the program counter
unsigned char readNextByte() {
  unsigned char byte = memory[cpu.REG_PC];
  cpu.REG_PC++;
  return byte;
}

void readInstruction() {
  // Read opcode
  unsigned char opcode = readNextByte();

  Addr_mode address_mode;
  // Get address mode by opcode
  switch (opcode) {
    case 0x0A:
    case 0x2A:
    case 0x4A:
    case 0x6A:
      address_mode = accumulator;
      break;

    case 0x00:
    case 0x08:
    case 0x18:
    case 0x28:
    case 0x38:
    case 0x40:
    case 0x48:
    case 0x58:
    case 0x60:
    case 0x68:
    case 0x78:
    case 0x88:
    case 0x8A:
    case 0x98:
    case 0x9A:
    case 0xA8:
    case 0xAA:
    case 0xB8:
    case 0xBA:
    case 0xC8:
    case 0xCA:
    case 0xD8:
    case 0xE8:
    case 0xEA:
    case 0xF8:
      address_mode = implied;
      break;

    case 0x09:
    case 0x29:
    case 0x49:
    case 0x69:
    case 0xA0:
    case 0xA2:
    case 0xA9:
    case 0xC0:
    case 0xC9:
    case 0xE0:
    case 0xE9:
      address_mode = immediate;
      break;

    case 0x10:
    case 0x30:
    case 0x50:
    case 0x70:
    case 0x90:
    case 0xb0:
    case 0xd0:
    case 0xf0:
      address_mode = relative;
      break;
    
    case 0x05:
    case 0x06:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x45:
    case 0x46:
    case 0x65:
    case 0x66:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xE4:
    case 0xE5:
    case 0xE6:
      address_mode = zero_page;
      break;

    case 0x15:
    case 0x16:
    case 0x35:
    case 0x36:
    case 0x55:
    case 0x56:
    case 0x75:
    case 0x76:
    case 0x94:
    case 0x95:
    case 0xB4:
    case 0xB5:
    case 0xD5:
    case 0xD6:
    case 0xF5:
    case 0xF6:
      address_mode = zero_page_idx_X;
      break;

    case 0x96:
    case 0xb6:
      address_mode = zero_page_idx_Y;
      break;

    case 0x0D:
    case 0x0E:
    case 0x20:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x6D:
    case 0x6E:
    case 0x8C:
    case 0x8D:
    case 0x8E:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xEC:
    case 0xED:
    case 0xEE:
      address_mode = absolute;
      break;

    case 0x1D:
    case 0x1E:
    case 0x3D:
    case 0x3E:
    case 0x5D:
    case 0x5E:
    case 0x7D:
    case 0x7E:
    case 0x9D:
    case 0xBC:
    case 0xBD:
    case 0xDD:
    case 0xDE:
    case 0xFD:
    case 0xFE:
      address_mode = absolute_idx_X;
      break;

    case 0x19:
    case 0x39:
    case 0x59:
    case 0x79:
    case 0x99:
    case 0xB9:
    case 0xBE:
    case 0xD9:
    case 0xF9:
      address_mode = absolute_idx_Y;
      break;

    case 0x6C:
      address_mode = absolute_indirect;
      break;

    case 0x11:
    case 0x31:
    case 0x51:
    case 0x71:
    case 0x91:
    case 0xB1:
    case 0xD1:
    case 0xF1:
      address_mode = zero_page_indirect_idx_Y;
      break;

    case 0x01:
    case 0x21:
    case 0x41:
    case 0x61:
    case 0x81:
    case 0xA1:
    case 0xC1:
    case 0xE1:
      address_mode = zero_page_idx_indirect_X;
      break;

  }

  switch (opcode)
  {
    // ORA
    case 0x01:
    case 0x05:
    case 0x09:
    case 0x0D:
    case 0x11:
    case 0x15:
    case 0x19:
    case 0x1D:
      // TODO: Implement ORA
      break;

    case 0x00: // BRK
      setFlag(FLAG_B);
      setFlag(FLAG_I);
      cpu.isRunning = 0;
      break;

    // LDA
    case 0xA1:
    case 0xA5:
    case 0xA9:
    case 0xAD:
    case 0xB1:
    case 0xB5:
    case 0xB9:
    case 0xBD:
      load(&cpu.REG_A, address_mode, readNextByte());
      modifyNegativeFlag(cpu.REG_A);
      modifyZeroFlag(cpu.REG_A);
      break;

    // LDX
    case 0xA2:
    case 0xA6:
    case 0xAE:
    case 0xB6:
    case 0xBE:
      load(&cpu.REG_X, address_mode, readNextByte());
      modifyNegativeFlag(cpu.REG_X);
      modifyZeroFlag(cpu.REG_X);
      break;

    // LDY
    case 0xA0:
    case 0xA4:
    case 0xAC:
    case 0xB4:
    case 0xBC:
      load(&cpu.REG_Y, address_mode, readNextByte());
      modifyNegativeFlag(cpu.REG_Y);
      modifyZeroFlag(cpu.REG_Y);
      break;

    // -- Store functions --

    // STA
    case 0x81:
    case 0x85:
    case 0x8D:
    case 0x91:
    case 0x95:
    case 0x99:
    case 0x9D:
      store(cpu.REG_A, address_mode, readNextByte());
      break;

    // STX
    case 0x86:
    case 0x8E:
    case 0x96:
      store(cpu.REG_X, address_mode, readNextByte());
      break;

    // STY
    case 0x84:
    case 0x8C:
    case 0x94:
      store(cpu.REG_Y, address_mode, readNextByte());
      break;

    // -- Increment and decrement --

    // INC
    case 0xE6:
    case 0xEE:
    case 0xF6:
    case 0xFE:
      incrementInMemory(address_mode, readNextByte());
      break;

    // INX
    case 0xE8:
      cpu.REG_X++;
      modifyNegativeFlag(cpu.REG_X);
      modifyZeroFlag(cpu.REG_X);
      break;

    // INY
    case 0xC8:
      cpu.REG_Y++;
      modifyNegativeFlag(cpu.REG_Y);
      modifyZeroFlag(cpu.REG_Y);
      break;

    // DEC
    case 0xC6:
    case 0xCE:
    case 0xD6:
    case 0xDE:
      decrementInMemory(address_mode, readNextByte());
      break;
    
    // DEX
    case 0xCA:
      cpu.REG_X--;
      modifyNegativeFlag(cpu.REG_X);
      modifyZeroFlag(cpu.REG_X);
      break;

    // DEY
    case 0x88:
      cpu.REG_Y--;
      modifyNegativeFlag(cpu.REG_Y);
      modifyZeroFlag(cpu.REG_Y);
      break;

  }
}

int runCPU() {
  resetCPU();

  while (cpu.isRunning) {
    readInstruction();
  }
  return 0;
}

// int main() {
//   cpu.initCPU = initCPU;
//   cpu.runCPU = runCPU;
//   return 0;
// }
