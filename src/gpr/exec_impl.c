#include "exec.h"
#include "gpr/instr.h"
#include "mem.h"

#include <stdio.h>

int exec_run(uint32_t start, uint32_t text, uint32_t data){
  uint32_t pc = start, running = 1;
  union gpr_instr_t ir;

  while(running){
    ir.u = mem_read32(pc);
    pc += 4;

    switch(ir.r.op){
    case SYSCALL:
      break;

    case LA:
      break;

    case LB:
      break;

    case LI:
      break;

    case B:
      break;

    case BEQZ:
      break;

    case BGE:
      break;

    case BNE:
      break;

    case ADDI:
      break;

    case SUBI:
      break;
    }
  }
}
