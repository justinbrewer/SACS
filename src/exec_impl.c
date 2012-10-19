#include "exec.h"
#include "instr.h"
#include "mem.h"

#include <stdio.h>

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, running = 1, reg[32] = {0};
  union gpr_instr_t ir;

  while(running){
    ir.u = mem_read32(pc);
    pc += 4;

    switch(ir.r.op){

    case SYSCALL:
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

    case NOP:
      break;

    case LA:
      reg[ir.i.rd] = data + ir.i.offset;
      break;

    case LB:
      reg[ir.i.rd] = mem_read8(reg[ir.i.rs] + ir.i.offset);
      break;

    case LI:
      reg[ir.i.rd] = ir.i.offset;
      break;

    case B:
      pc += ir.j.offset<<2;
      break;

    case BEQZ:
      if(reg[ir.i.rs] == 0){
	pc += ir.i.offset<<2;
      }
      break;

    case BGE:
      if(reg[ir.i.rd] >= reg[ir.i.rs]){
	pc += ir.i.offset<<2;
      }
      break;

    case BNE:
      if(reg[ir.i.rd] != reg[ir.i.rs]){
	pc += ir.i.offset<<2;
      }
      break;

    case ADDI:
      reg[ir.i.rd] = reg[ir.i.rs] + ir.i.offset;
      break;

    case SUBI:
      reg[ir.i.rd] = reg[ir.i.rs] - ir.i.offset;
      break;
    }

    reg[0] = 0; //This sucks
  }

  return 0;
}
