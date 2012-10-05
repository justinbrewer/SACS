#include "exec.h"
#include "gpr/instr.h"
#include "mem.h"

#include <stdio.h>

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  int ic = 0, c = 0;
  uint32_t pc = start, running = 1, reg[32] = {0};
  union gpr_instr_t ir;

  while(running){
    ic++;
    ir.u = mem_read32(pc);
    pc += 4;

    switch(ir.r.op){

    case SYSCALL:
      c += 8;
      switch(reg[2]){
      case 4:
	fputs((char*)mem_translate_addr(reg[4]), stdout);
	break;

      case 8:
	fgets((char*)mem_translate_addr(reg[4]), reg[5], stdin);
	break;

      case 10:
	running = 0;
	break;
      }

      break;

    case LA:
      c += 5;
      reg[ir.i.rd] = data + ir.i.offset;
      break;

    case LB:
      c += 6;
      reg[ir.i.rd] = mem_read8(reg[ir.i.rs] + ir.i.offset);
      break;

    case LI:
      c += 3;
      reg[ir.i.rd] = ir.i.offset;
      break;

    case B:
      c += 4;
      pc += ir.j.offset<<2;
      break;

    case BEQZ:
      c += 5;
      if(reg[ir.i.rs] == 0){
	pc += ir.i.offset<<2;
      }
      break;

    case BGE:
      c += 5;
      if(reg[ir.i.rd] >= reg[ir.i.rs]){
	pc += ir.i.offset<<2;
      }
      break;

    case BNE:
      c += 5;
      if(reg[ir.i.rd] != reg[ir.i.rs]){
	pc += ir.i.offset<<2;
      }
      break;

    case ADDI:
      c += 6;
      reg[ir.i.rd] = reg[ir.i.rs] + ir.i.offset;
      break;

    case SUBI:
      c += 6;
      reg[ir.i.rd] = reg[ir.i.rs] - ir.i.offset;
      break;
    }

    reg[0] = 0; //This sucks
  }

  printf("\n=== Statistics ===\n\nC: %i\nIC: %i\n",c,ic);

  return 0;
}
