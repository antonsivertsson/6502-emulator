#include "stdio.h"
#include "memory.h"
#include "6502.h"

typedef struct {
  unsigned short PC;
  unsigned char PS;
  unsigned char SP;
  unsigned char A;
  unsigned char X;
  unsigned char Y;
} Expected;

int assert(char opcode[], Expected expected) {
  runCPU();

  unsigned char assert_PC = cpu.REG_PC == expected.PC;
  unsigned char assert_PS = cpu.REG_PS == expected.PS;
  unsigned char assert_SP = cpu.REG_SP == expected.SP;
  unsigned char assert_A = cpu.REG_A == expected.A;
  unsigned char assert_X = cpu.REG_X == expected.X;
  unsigned char assert_Y = cpu.REG_Y == expected.Y;

  if (!assert_PC) {
    printf("%s - assert PC FAILED: expected '%x' got '%x'\n", opcode, expected.PC, cpu.REG_PC);
  }
  else if (!assert_PS) {
    printf("%s - assert PS FAILED: expected '%x' got '%x'\n", opcode, expected.PS, cpu.REG_PS);
  }
  else if (!assert_SP) {
    printf("%s - assert SP FAILED: expected '%x' got '%x'\n", opcode, expected.SP, cpu.REG_SP);
  }
  else if (!assert_A) {
    printf("%s - assert A FAILED: expected '%x' got '%x'\n", opcode, expected.A, cpu.REG_A);
  }
  else if (!assert_X) {
    printf("%s - assert X FAILED: expected '%x' got '%x'\n", opcode, expected.X, cpu.REG_X);
  }
  else if (!assert_Y) {
    printf("%s - assert Y FAILED: expected '%x' got '%x'\n", opcode, expected.Y, cpu.REG_Y);
  } else {
    printf("%s - PASSED\n", opcode);
  }

  return (assert_PC && assert_PS && assert_SP && assert_A && assert_X && assert_Y);
}

int test_NOP() {
  memory[0x00] = 0x00; // Program counter will go to 0x00 from reset vector
  Expected expected = {
      0x01,         // PC
      0b00111100,   // PS - expect B & I since we let it run through all instructions
      0x00,         // SP
      0x00,         // A
      0x00,         // X
      0x00          // Y
    };
  return assert(
      "NOP",
      expected);
}

int test_LDA_immediate() {
  memory[0x00] = 0xa9; // LDA #oper
  memory[0x00 + 1] = 0x10;
  Expected expected = {
      0x03,         // PC
      0b00111100,   // PS - expect B & I since we let it run through all instructions
      0x00,         // SP
      0x10,         // A
      0x00,         // X
      0x00          // Y
  };
  return assert(
      "LDA_#",
      expected);
}

int main() {

  test_NOP();
  test_LDA_immediate();

  return 0;
}