#ifndef CPU_H
#define CPU_H

// Enums
typedef enum
{
  ADC = 0,
  AND,
  ASL,
  BCC,
  BCS,
  BEQ,
  BIT,
  BMI,
  BNE,
  BPL,
  BRK,
  BVC,
  BVS,
  CLC,
  CLD,
  CLI,
  CLV,
  CMP,
  CPX,
  CPY,
  DEC,
  DEX,
  DEY,
  EOR,
  INC,
  INX,
  INY,
  JMP,
  JSR,
  LDA,
  LDX,
  LDY,
  LSR,
  NOP,
  ORA,
  PHA,
  PHP,
  PLA,
  PLP,
  ROL,
  ROR,
  RTI,
  RTS,
  SBC,
  SEC,
  SED,
  SEI,
  STA,
  STX,
  STY,
  TAX,
  TAY,
  TSX,
  TXA,
  TXS,
  TYA,
}
Opcode;

// Addressing modes
typedef enum {
  implied = 0,
  accumulator,
  immediate,
  absolute,
  zero_page,
  relative,
  absolute_indirect,
  absolute_idx_X,
  absolute_idx_Y,
  zero_page_idx_X,
  zero_page_idx_Y,
  zero_page_idx_indirect_X,
  zero_page_indirect_idx_Y,
} Addr_mode;


// Structs
typedef struct {
  unsigned short REG_PC;
  unsigned char REG_SP;
  unsigned char REG_A;
  unsigned char REG_X;
  unsigned char REG_Y;
  unsigned char REG_PS;

  unsigned isRunning;

  int (*initCPU)();
  int (*runCPU)();
} CPU;

typedef struct {
  Opcode operation;
  Addr_mode address_mode;
  unsigned char operator;
} Instruction;

// Functions
int resetCPU();
int runCPU();

extern CPU cpu;

#endif