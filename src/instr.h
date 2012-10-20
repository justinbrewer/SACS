#pragma once

#include <stdint.h>

typedef enum { NOP=0x00, SYSCALL=0x01,
	       LA=0x10, LB=0x11, LI=0x12,
	       B=0x20, BEQZ=0x21, BGE=0x22, BNE=0x23,
	       ADDI=0x30, SUBI=0x31,
             } gpr_op_t;

struct gpr_instr_r_t {
  unsigned int op    : 6;
  unsigned int rs    : 5;
  unsigned int rt    : 5;
  unsigned int rd    : 5;
  unsigned int shift : 5;
  unsigned int func  : 6;
};

struct gpr_instr_i_t {
  unsigned int op     :  6;
  unsigned int rs     :  5;
  unsigned int rd     :  5;
  signed   int offset : 16;
};

struct gpr_instr_j_t {
  unsigned int op     :  6;
  signed   int offset : 26;
};

union gpr_instr_t {
  uint32_t u;
  struct gpr_instr_r_t r;
  struct gpr_instr_i_t i;
  struct gpr_instr_j_t j;
};
