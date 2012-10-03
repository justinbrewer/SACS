#pragma once

typedef enum { SYSCALL=0x00,
	       LA=0x10, LB=0x11, LI=0x12,
	       B=0x20, BEQZ=0x21, BGE=0x22, BNE=0x23,
	       ADDI=0x30, SUBI=0x31,
             } gpr_instr;

struct gpr_instr_r {
  unsigned int op    : 6;
  unsigned int rs    : 5;
  unsigned int rt    : 5;
  unsigned int rd    : 5;
  unsigned int shift : 5;
  unsigned int func  : 6;
};

struct gpr_instr_i {
  unsigned int op     :  6;
  unsigned int rs     :  5;
  unsigned int rd     :  5;
  unsigned int offset : 16;
};

struct gpr_instr_j {
  unsigned int op   :  6;
  unsigned int addr : 26;
};
